#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>

void clearInputBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

char *ltrim(char *str)
{
    while (isspace((unsigned char)*str))
    {
        str++;
    }
    return str;
}

char *rtrim(char *str)
{
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
    {
        end--;
    }
    *(end + 1) = '\0';
    return str;
}

char *trim(char *s)
{
    return rtrim(ltrim(s));
}

void history(char *command)
{
    FILE *file = fopen(".myBash_history.txt", "r+");
    if (file == NULL)
    {
        perror("Error opening history file");
        return;
    }

    char history[20][1000];
    int lineCount = 0;
    while (fgets(history[lineCount], 1000, file) != NULL && lineCount < 20)
    {
        lineCount++;
    }

    if (lineCount == 20)
    {

        fclose(file);
        FILE *file = fopen(".myBash_history.txt", "w+");
        if (file == NULL)
        {
            perror("Error opening history file");
            return;
        }

        for (int i = 0; i < 19; i++)
        {
            fprintf(file, "%s", history[i + 1]);
        }
    }

    fprintf(file, "%s\n", command);
    fclose(file);
}

void handle_sigquit(int sig)
{
    printf("\nHistory:\n");

    FILE *file = fopen(".myBash_history.txt", "r");
    if (file == NULL)
    {
        printf("Error opening history file.\n");
        exit(1);
    }

    char line[1000];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        printf("%s", line);
    }

    fclose(file);

    exit(0);
}

int main(int argc, char *argv[])
{
    // FILE *file;
    int file;
    int script = 0;

    signal(SIGQUIT, handle_sigquit);

    if (argc == 2)
    {
        script = 1;
        /*file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Error opening file %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }
        char buffer[1000];
        fscanf(file, "%s", buffer);
        getc();*/
        // fscanf(file,"%999[^\n]",buffer);

        file = open(argv[1], O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (file == -1)
        {
            perror("open");
            exit(EXIT_FAILURE);
        }
        dup2(file, STDIN_FILENO);
    }
    int firstLine = 1;
    pid_t pid = 0;

    while (1)
    {
        char str[1000] = "";
        char *commands[100];
        int outputs[100];
        int comm_count = 0;
        int pos = 0;
        char command[100] = "";

        for (int i = 0; i < 100; i++)
        {
            outputs[i] = 0;
            commands[i] = NULL;
        }

        int help = 0;
        printf("> ");

        if (script && firstLine)
        {
            help = scanf("%999[^\n]", str);
            firstLine = 0;
        }

        help = scanf("%999[^\n]", str);
        
        int status;
        if(pid>0)waitpid(pid, &status, WNOHANG);
        if(status)printf("\n> ");
            
        // printf("%s\n", str);

        if (help == EOF && script)
        {
            exit(EXIT_SUCCESS);
        }
        clearInputBuffer();
        if (help != 1)
            continue;
        // }

        history(str);

        while (pos < strlen(str) && sscanf(str + pos, "%[^|]", command) == 1 && comm_count < 100)
        {
            commands[comm_count] = strdup(command);
            comm_count++;
            pos += strlen(command) + 1;
            while (str[pos] == '|')
                pos++;
        }

        int background = 0;
        char last = commands[comm_count - 1][strlen(commands[comm_count - 1]) - 1];
        if (last == '&')
        {
            background = 1;
            commands[comm_count - 1][strlen(commands[comm_count - 1]) - 1] = '\0';
        }

        int **pipes;

        pipes = (int **)malloc((comm_count - 1) * sizeof(int *));
        for (int i = 0; i < comm_count - 1; i++)
            pipes[i] = (int *)malloc(2 * sizeof(int));

        for (int i = 0; i < comm_count - 1; i++)
        {
            if (pipe(pipes[i]) == -1)
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < comm_count; i++)
        {
            char *tempCommand = strtok(commands[i], ">>");
            char *outputFile;
            if (tempCommand != NULL)
            {
                outputFile = strtok(NULL, ">>");

                if (outputFile != NULL)
                {
                    outputFile = trim(outputFile);
                    outputs[i] = 1;
                }
            }

            char *token = strtok(tempCommand, " ");
            if (token == NULL)
            {
                continue; // Empty line
            }

            char *commando = token;
            char *args[100];
            args[0] = commando;

            int a = 1;
            while ((token = strtok(NULL, " ")) != NULL && a < 100)
            {
                args[a++] = token;
            }
            args[a] = NULL;

            if (strcmp(args[0], "cd") == 0)
            {
                if (args[1] == NULL)
                {
                    fprintf(stderr, "cd: missing argument\n");
                }
                else
                {
                    if (chdir(args[1]) != 0)
                    {
                        perror("cd");
                    }
                }
                break;
            }
            pid = fork();

            if (pid < 0)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0)
            {
                if (outputs[i])
                {
                    int file_fd = open(outputFile, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    if (file_fd == -1)
                    {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    dup2(file_fd, STDOUT_FILENO);
                }

                if (i == 0)
                { // First child process
                    if (i != comm_count - 1)
                    {
                        close(pipes[i][0]);
                        if (outputs[i])
                            close(pipes[i][1]);
                        else
                            dup2(pipes[i][1], STDOUT_FILENO);

                        for (int j = 0; j < comm_count - 1; j++)
                        {
                            close(pipes[j][0]);
                            close(pipes[j][1]);
                        }
                    }

                    execvp(commando, args);
                }
                else if (i == comm_count - 1)
                { // Last child process
                    close(pipes[i - 1][1]);
                    if (outputs[i - 1])
                        close(pipes[i - 1][0]);
                    else
                        dup2(pipes[i - 1][0], STDIN_FILENO);

                    for (int j = 0; j < comm_count - 1; j++)
                    {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }

                    execvp(commando, args);
                }
                else
                { // Intermediate child processes
                    close(pipes[i - 1][1]);
                    if (outputs[i - 1])
                        close(pipes[i - 1][0]);
                    else
                        dup2(pipes[i - 1][0], STDIN_FILENO);

                    close(pipes[i][0]);
                    if (outputs[i])
                        close(pipes[i][1]);
                    else
                        dup2(pipes[i][1], STDOUT_FILENO);

                    for (int j = 0; j < comm_count - 1; j++)
                    {
                        if (j != i - 1 && j != i)
                        {
                            close(pipes[j][0]);
                            close(pipes[j][1]);
                        }
                    }

                    execvp(commando, args);
                }
                exit(EXIT_SUCCESS);
            }
        }
        for (int i = 0; i < comm_count - 1; i++)
        {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        if (background == 0)
        {
            for (int i = 0; i < comm_count; i++)
            {
                wait(NULL);
            }
        }
        else
        {
            background = 0;
        }
    }

    return 0;
}