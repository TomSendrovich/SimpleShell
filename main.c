#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <pwd.h>

char msg[100], path[100] = {NULL};
__pid_t pid = -2, allPids[100], parentPID;
int countTokens = 0, countPids = 0, returnCode;
char *commands[10], *token = NULL;
char **history[100];
char *msgSave;
bool isChild = false;

void printAndGetFromCmd() {
    sleep(0.2);
    printf("> ");
    gets(msg);
    msgSave = (char *) malloc((strlen(msg) + 1) * sizeof(char));
    strcpy(msgSave, msg);
}

//Dividing into commands and putting it in the 'commands' array
void divideToCommands() {

    token = strtok(msg, " ");
    char newToken[100];
    while (token != NULL) {
        if (token[0] == '"') {
            token++;
            //   strcpy(newToken,token);
            //  token=newToken;
        }
        if (token[strlen(token) - 1] == '"') {

            strncpy(newToken, token, strlen(token) - 1);
            strcpy(token, newToken);
        }
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

        for (i = 0; i < countPids; i++) {
            if ((kill(allPids[i], 0)) != 0 || strstr(history[i], "cd") != NULL) {
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
            if ((kill(allPids[i], 0)) == 0 && strstr(history[i], "cd") == NULL) {
                printf("%d %s\n", allPids[i], history[i]);
            }
        }
    } else {
        wait(NULL);
    }
}

void cdCommand() {
    pid = getpid();
    printf("%d\n", pid);
    if (commands[2] != NULL) {
        fprintf(stderr, "Error: Too many arguments\n");
    } else if (strcmp(commands[1], "..") == 0) {
        getcwd(path, 100);
        chdir("..");
    } else if (strcmp(commands[1], "-") == 0) {
        if (path[0]=='\0'){
            fprintf(stderr,"OLDPWD not set\n");
        } else{
            chdir(path);
        }

    } else if (strcmp(commands[1], "~") == 0) {
        getcwd(path, 100);
        struct passwd *pw = getpwuid(getuid());
        const char *homedir = pw->pw_dir;
        chdir(homedir);
    } else if (chdir(commands[1]) != 0) {
        fprintf(stderr, "Error: No such file or directory\n");
    } else {
        getcwd(path, 100);
        chdir(commands[1]);
    }


}

void backgroundProcess() {
    commands[countTokens - 1] = NULL;
    pid = fork();
    if(pid<0)
    {
        fprintf(stderr, "Error in system call\n");
        exit(-1);
    }else if (pid == 0) {
        //child
        isChild = true;
        pid = getpid();
        printf("%d", pid);//print pid
        printf("\n");
        if ((returnCode = execvp(commands[0], commands) == -1)) {
            fprintf(stderr, "Error in system call\n");

        }
    }
}

void foregroundProcess() {
    pid = fork();
    if(pid<0)
    {
        fprintf(stderr, "Error in system call");
        exit(-1);
    }else if (pid == 0) {
        //child
        isChild = true;
        pid = getpid();
        printf("%d\n", pid);

        if ((returnCode = execvp(commands[0], commands) == -1)) {
            fprintf(stderr, "Error in system call\n");
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
