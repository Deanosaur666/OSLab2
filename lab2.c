#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define CHILDCOUNT 16

int main() {
    // the commands each child will execute
    // NULL indicates aborting
    char* cargs[][8] = {
        { "echo", "Hello Dean Yockey", NULL},
        { NULL }, // abort
        { "ls", "-l", NULL},
        { "date", NULL },
        { "rm", "..", NULL }, // should fail
        { "ps", NULL },
        { "stat", "lab2.c", NULL },
        { "time", "pwd", NULL},
        { "mkdir", "pointless", NULL}, // will fail if it already exists
        { "rmdir", "pointless", NULL}, // will fail if it hasn't been created yet
        { "touch", "-c", "notreal.txt", NULL},
        { "wc", "-l", "lab2.c", NULL },
        { NULL }, // abort
        { "whoami", NULL },
        { "less", "notreal.txt", NULL }, // should fail
        { "tty", NULL }
    };
    // parent's PID
    pid_t ppid = getpid();
    printf("Welcome to process %d!\n", ppid);

    // child pid
    pid_t pid;
    pid_t children[CHILDCOUNT];

    // recording exit types for summary
    int zeroExits = 0;
    int nonzeroExits = 0;
    int signalExits = 0;

    int c; // c can be used to identify which child I am

    // create 15 children
    for(c = 0; c < CHILDCOUNT; c ++) {
        pid = fork();
        if(pid == 0)    // children don't need children
            break;      // if we didn't break, we'd get an 
                        // infinite loop of children creating more children
        else if(pid < 0) { // error
            fprintf(stderr, "Fork failed!\n");
            return 1;
        }
        children[c] = pid;
    }

    // child
    if(pid == 0) {
        pid_t cpid = getpid();
        char** args = cargs[c];
        
        // abort
        if(args[0] == NULL) {
            printf("Child %d (PID: %d) will abort.\n", c, cpid);
            abort();
        }
        
        // run command with execvp
        else {
            printf("Child %d (PID: %d) will execute command \"%s.\"\n", c, cpid, args[0]);
            execvp(args[0], args);
        }
    }
    // parent
    else {
        // wait for each child in order
        for(int i = 0; i < CHILDCOUNT; i ++) {
            int status;
            waitpid(children[i], &status, 0);

            // exit status
            if(WIFEXITED(status)) {
                printf("Child %d exit status: %d\n", i, WEXITSTATUS(status));
                if(WEXITSTATUS(status) == 0)
                    zeroExits ++;
                else
                    nonzeroExits ++;
            }
            // termination signal
            else if(WIFSIGNALED(status)) {
                printf("Child %d exited with signal: %d\n", i, WTERMSIG(status));
                signalExits ++;
            }
        }

        // summary
        printf("====================\n");
        printf("      SUMMARY\n");
        printf("====================\n");
        printf("%d children exited with exit code 0.\n", zeroExits);
        printf("%d children exited with non-zero exit code.\n", nonzeroExits);
        printf("%d children were terminated by a signal.\n", signalExits);
    }
    

    return 0;
}