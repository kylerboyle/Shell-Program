#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void sigint_handler(int sig) {
    char message[] = "SIGINT handled\n";
    char prompt[] = "361> ";
    write(1, message, sizeof(message));
    write(1, prompt, sizeof(prompt));
}

void sigstp_handler(int sig) {
    char message[] = "SIGSTP handled\n";
    char prompt[] = "361> ";
    write(1, message, sizeof(message));
    write(1, prompt, sizeof(prompt));
}

// check which type/if redirection is used
int isRedirection(char* word) {
    if(strcmp(word, ">") == 0) {
        return 1;
    } else if(strcmp(word, ">>") == 0) {
        return 2;
    } else if(strcmp(word, "<") == 0) {
        return 3;
    } else {
        return 0;
    }
}

int main() {
    
    // signal handlers
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigstp_handler);
    
    char buffer[500];
    char **args = malloc(100 * sizeof(char*));
    char *filename;
    
    //print initial prompt
    printf("361> ");
    
    // get input in loop until null or terminated
    while(fgets(buffer, 500, stdin)) {

        // parse the input
        char *word = strtok(buffer, " ");
        int i = 0;
        int redirectionFlag = 0;
        while(word) {
            char *newlineChar = strchr(word, '\n');
            if(newlineChar) {
                *newlineChar = 0;
            }
            if(redirectionFlag) {
                filename = word;
                break;
            }
            if(strcmp(word, "exit") == 0) {
                printf("terminating session...\n");
                return 0;
            }
            
            redirectionFlag = isRedirection(word);
            if(redirectionFlag) {
                // do nothing
            } else {
                args[i++] = word;
            }
            // get next word
            word = strtok(NULL, " ");
        }

        args[i] = NULL;
        
        int pid = fork();
        if(pid == 0) {
            if(redirectionFlag == 1) {
                int fd1 = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0666);
                dup2(fd1, 1);
                close(fd1);
            } else if(redirectionFlag == 2) {
                int fd1 = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0666);
                dup2(fd1, 1);
                close(fd1);
            } else if(redirectionFlag == 3) {
                int fd1 = open(filename, O_RDONLY);
                dup2(fd1, 0);
                close(fd1);
            }
            execvp(args[0], args);
            exit(6);
        } else {
            int status;
            printf("PID: %d\n", pid);
            wait(&status);
            printf("Exit: %d\n", WEXITSTATUS(status));
        }
        printf("361> ");
    }
    return 0;
}
