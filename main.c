#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>

char msg[100], path[100];
__pid_t pid = -2, allPids[100], parentPID;
int countTokens = 0, countPids = 0, returnCode;
char *commands[10], *token = NULL;
char **history[100];
char *msgSave;
bool isChild = false;

void printAndGetFromCmd() {
    printf("> ");
    gets(msg);
    // printf("my PID is: %d. status: %d (scan)\n", getpid(), kill(getpid(), 0));
    msgSave = (char *) malloc((strlen(msg) + 1) * sizeof(char));
    strcpy(msgSave, msg);
}

void divideToCommands() {
    //Dividing into commands and putting it in the 'commands' array
    token = strtok(msg, " ");
    while (token != NULL) {
        commands[countTokens] = token;
        countTokens++;
        token = strtok(NULL, " ");
    }
    commands[countTokens] = NULL;
}

void historyCommand() {
    int i;
    if ((pid = fork()) == 0) {
        isChild = true;
        allPids[countPids] = getpid();
        countPids++;

        printf("number of processes: %d\n", countPids);

        for (i = 0; i < countPids; i++) {
            if ((kill(allPids[i], 0)) != 0) {
                printf("%d %s DONE\n", allPids[i], history[i]);
            } else {
                printf("%d %s RUNNING\n", allPids[i], history[i]);
            }
        }
    } else {
        wait(NULL);
    }
}

void jobsCommand() {
    int i = 0;
    if ((pid = fork()) == 0) {
        isChild = true;
        for (i = 0; i < countPids; i++) {
            if ((kill(allPids[i], 0)) == 0) {
                printf("%d %s\n", allPids[i], history[i]);
            }
        }
    } else {
        wait(NULL);
    }
}

int findLastSlash() {
    int i = 0, last = 0;
    while (path[i] != '\0') {
        if (path[i] == '/') {
            last = i;
        }
        i++;
    }
    return last;
}

void cdCommand() {

    if (commands[2] != NULL) {
        fprintf(stderr, "Error: Too many arguments\n");
    }else if (strcmp(commands[1], "..") == 0) {
        /*  getcwd(path,100);
          int last = findLastSlash();
          char newPath[100];
          strncpy(newPath,path,last);*/
        getcwd(path, 100);

        chdir("..");
    }else if (strcmp(commands[1], "-") == 0) {
        chdir(path);
    } else if (strcmp(commands[1], "~") == 0) {
        getcwd(path, 100);
        chdir("/home");
    } else if (chdir(commands[1]) != 0) {
        fprintf(stderr, "Error: No such file or directory\n");
    } else {
        getcwd(path, 100);
        chdir(commands[1]);
    }

    pid = getpid();
    printf("%d\n", pid);
}

void backgroundProcess() {
    commands[countTokens - 1] = NULL;
    if ((pid = fork()) == 0) {
        //child
        isChild = true;
        pid = getpid();
        printf("%d", pid);//print pid
        printf("\n");
        if ((returnCode = execvp(commands[0], commands) == -1)) {
            printf("exec failed");
            printf("\n");
        }
    }
}

void foregroundProcess() {
    if (strcmp(commands[0],"echo")==0){

    }
    if ((pid = fork()) == 0) {
        //child
        isChild = true;
        pid = getpid();
        printf("%d", pid);
        printf("\n");
        if ((returnCode = execvp(commands[0], commands) == -1)) {
            printf("exec failed");
            printf("\n");
        }
    } else {
        wait(&returnCode);
    }
}

int main() {

    parentPID = getpid();
    isChild = false;
    while (true) {
        printAndGetFromCmd();

        history[countPids] = msgSave;

        divideToCommands();

        if (strcmp(commands[0], "exit") == 0) {
            return 0;
        } else if (strcmp(commands[0], "history") == 0) {
            historyCommand();
        } else if (strcmp(commands[0], "jobs") == 0) {
            jobsCommand();
        } else if (strcmp(commands[0], "cd") == 0) {
            cdCommand();
        } else if (strcmp(commands[countTokens - 1], "&") == 0) {
            backgroundProcess();
        } else {
            foregroundProcess();
        }

        if (isChild == true) {
            return 0;
        }

        countTokens = 0;
        //save te pid number and process name

        allPids[countPids] = pid;

        // history[countPids] = msg;
        countPids++;
    }
}
