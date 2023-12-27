#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>


int myPipe;
int myLog;

void terminateHandler(int sig){
    if(sig == SIGUSR1){
        close(myPipe);
        close(myLog);
        exit(0);
    }
}

int isStringValid(char string[], char *validStrings[]){
    for(int i = 0; validStrings[i] != NULL; i++){
        if(strcmp(string, validStrings[i]) == 0) return 1;
    } return 0;
}

int main(){

    signal(SIGUSR1, terminateHandler);

    int count;
    char msg[9] = "";
    char prevMsg[9] = "";
    char *validStrings[] = {"DESTRA", "SINISTRA", NULL};
    char msgNoAct[] = "NO ACTION\n";
    char logMsg[24];

    myPipe = open("./pipe/steerPipe", O_RDONLY | O_NONBLOCK);
    if(myPipe == -1){
        perror("sw");
        exit(-1);
    }

    myLog = open("./log/steer.log", O_WRONLY | O_TRUNC);
    if(myLog == -1){
        perror("sw");
        exit(-1);
    }

    strcpy(logMsg, msgNoAct);

    while(1){
        read(myPipe, msg, sizeof(msg));

        // controlla se il msg è DESTRA o SINISTRA e che non sia uguale al precedente
        if(isStringValid(msg, validStrings) && strcmp(prevMsg, msg) != 0){
            count = 0;
            strcpy(prevMsg, msg);
            sprintf(logMsg, "STO GIRANDO A %s\n", msg);
        }
        
        if(count == 4){
            count = 0;
            strcpy(prevMsg, "");
            strcpy(logMsg, msgNoAct);   
        }
        count++;

        write(myLog, logMsg, strlen(logMsg));

        sleep(1);
    }

    return 0;
}