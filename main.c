#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main ()
{
    int fds[2];
    pid_t pid;
    /* Create a pipe. File descriptors for the two ends of the pipe are placed in fds. */
    /* TODO add error handling for system calls like pipe, fork, etc. */
    pipe (fds);
    /* Fork a child process. */
    pid = fork ();
    if (pid == (pid_t) 0) {
        /* This is the child process. Close our copy of the write end of the file descriptor. */
        close (fds[1]);
        /* Connect the read end of the pipe to standard input. */
        dup2 (fds[0], STDIN_FILENO);
        /* Replace the child process with the "sort” program. */
        execlp ("sort", "sort", NULL);
    } else {
        while(1){
            char str[1000];
            //char delimiters[] = "|"; // Delimiters include space, comma, period, exclamation mark, and question mark
            char *commands[100]; // Array to store words
            int comm_count = 0;

            printf("> ");
            scanf("%999[^\n]",str);
            clearInputBuffer();

            int pos = 0;
            char command[100];
            // sscanf(str, "%[^|]", command);
            // printf("debug> %s\n", command);
            
            while (pos<strlen(str) && sscanf(str + pos, "%[^|]", command) == 1 && comm_count < 100) {
                // printf("Pos: %d, Word: %s\n",pos, command);
                commands[comm_count]=strdup(command);
                comm_count++;
                pos += strlen(command) + 1; 
                while (str[pos] == '|') pos++; 
            }
            
            /*printf("%d\n", comm_count);
            for (int i = 0; i < comm_count; i++) {
                printf("%s\n", commands[i]);
            }*/
        }
        
        /* This is the parent process. */
        FILE* stream;
        /* Close our copy of the read end of the file descriptor. */
        close (fds[0]);
        /* Convert the write file descriptor to a FILE object, and write to it. */
        stream = fdopen (fds[1], "w");
        fprintf (stream, "This is a test.\n");
        fprintf (stream, "Hello, world.\n");
        fprintf (stream, "My dog has fleas.\n");
        fprintf (stream, "This program is great.\n");
        fprintf (stream, "One fish, two fish.\n");
        fflush (stream);
        close (fds[1]);
        /* Wait for the child process to finish. */
        waitpid (pid, NULL, 0);
    }
    return 0;
}