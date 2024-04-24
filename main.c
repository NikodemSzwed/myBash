#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void history(char *command){
    FILE *file;
    char filename[] = ".myBash_history.txt";
    char buffer[10100]; 

    file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        return;
    }

    fgets(buffer, sizeof(buffer), file);
    printf("Content read from file: %s\n", buffer);

  
    fclose(file);

    char *firstEnter;

    firstEnter = strchr(buffer, '\n');
    char substring[10100];
    if (firstEnter != NULL) {
        
        strcpy(substring, firstEnter+1);
        
        printf("Substring after first newline character: %s\n", substring);
    } else {
        printf("No newline character found in the string.\n");
    }

    strcat(substring, command);

    file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        return;
    }

    fprintf(file,"%s\n", substring);
    fclose(file);


}

int main ()
{
    while(1){
        char str[1000];
        char *commands[100]; 
        int comm_count = 0;

        printf("> ");
        scanf("%999[^\n]",str);
        clearInputBuffer();
        history(str);

        int pos = 0;
        char command[100];
        
        while (pos<strlen(str) && sscanf(str + pos, "%[^|]", command) == 1 && comm_count < 100) {
            commands[comm_count]=strdup(command);
            comm_count++;
            pos += strlen(command) + 1; 
            while (str[pos] == '|') pos++;
        }

        int **pipes;

        pipes = (int **)malloc((comm_count - 1) * sizeof(int *));
        for(int i = 0; i < comm_count - 1; i++) pipes[i] = (int *)malloc(2 * sizeof(int));
        pid_t pid;

        for (int i = 0; i < comm_count - 1; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        for (int i=0;i<comm_count;i++){
            char *token = strtok(commands[i], " ");
            if (token == NULL) {
                continue; // Empty line
            }

            char *commando = token;
            char *args[100];
            args[0] = commando;

            int a = 1;
            while ((token = strtok(NULL, " ")) != NULL && a < 100) {
                args[a++] = token;
            }
            args[a] = NULL;

            if (strcmp(args[0], "cd") == 0) {
                if (args[1] == NULL) {
                    fprintf(stderr, "cd: missing argument\n");
                } else {
                    if (chdir(args[1]) != 0) {
                        perror("cd");
                    }
                }
                break;
            }
            pid = fork();

            if (pid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {  
                if (i == 0) {  // First child process
                    if(i!=comm_count-1){
                        close(pipes[i][0]);
                        dup2(pipes[i][1], STDOUT_FILENO);

                        for (int j = 0; j < comm_count - 1; j++) {
                            close(pipes[j][0]);
                            close(pipes[j][1]);
                        }
                    }

                    execvp(commando, args);
                } else if (i == comm_count - 1) {  // Last child process

                    close(pipes[i - 1][1]);
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                    for (int j = 0; j < comm_count - 1; j++) {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }

                    execvp(commando, args);
                } else {  // Intermediate child processes
                    close(pipes[i - 1][1]);
                    dup2(pipes[i - 1][0],STDIN_FILENO);
                    close(pipes[i][0]);
                    dup2(pipes[i][1], STDOUT_FILENO);

                    for (int j = 0; j < comm_count - 1; j++) {
                        if (j != i - 1 && j != i) {
                            close(pipes[j][0]);
                            close(pipes[j][1]);
                        }
                    }

                    execvp(commando, args);
                }
            }
        }
        for (int i = 0; i < comm_count - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        for (int i = 0; i < comm_count; i++) {
            wait(NULL);
        }
    }
   
    return 0;
}