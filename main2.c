#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define N 3

int main() {
    //int pipes[N - 1][2];
    int **pipes;//[comm_count - 1][2];

    pipes = (int **)malloc((N - 1) * sizeof(int *));
    for(int i = 0; i < N - 1; i++) pipes[i] = (int *)malloc(2 * sizeof(int));
    pid_t pid;

    // Create pipes
    for (int i = 0; i < N - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Create child processes
    for (int i = 0; i < N; i++) {
        pid = fork();

        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {  // Child process
            if (i == 0) {  // First child process
                // Close read end of first pipe
                if(i!=N-1){
                    close(pipes[i][0]);

                    // Redirect STDIN to the write end of the first pipe
                    dup2(pipes[i][1], STDOUT_FILENO);

                    // Close all other pipes
                    for (int j = 0; j < N - 1; j++) {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }
                }
                

                // Execute some command or program, for example, cat
                execlp("cat", "cat","tak.txt", NULL);
            } else if (i == N - 1) {  // Last child process
                // Close write end of last pipe
                close(pipes[i - 1][1]);

                // Redirect STDOUT to the read end of the last pipe
                dup2(pipes[i - 1][0], STDIN_FILENO);

                // Close all other pipes
                for (int j = 0; j < N - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                // Execute some command or program, for example, grep
                execlp("grep", "grep", "c", NULL);
            } else {  // Intermediate child processes
                // Close write end of previous pipe
                close(pipes[i - 1][1]);

                // Redirect STDIN to the read end of the previous pipe
                dup2(pipes[i - 1][0],STDIN_FILENO);

                // Close read end of current pipe
                close(pipes[i][0]);

                // Redirect STDOUT to the write end of the current pipe
                dup2(pipes[i][1], STDOUT_FILENO);

                // Close all other pipes
                for (int j = 0; j < N - 1; j++) {
                    if (j != i - 1 && j != i) {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }
                }

                // Execute some command or program, for example, sort
                execlp("sort", "sort", NULL);
            }
        }
    }

    // Close all pipes in the parent process
    for (int i = 0; i < N - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes to terminate
    for (int i = 0; i < N; i++) {
        wait(NULL);
    }

    return 0;
}