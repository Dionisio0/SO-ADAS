#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

struct message{
    char type[13];
    char data[8];
};

int myPipe;

void handleCloseInput(int sig){
    if(sig == SIGUSR1){
        printf("\n");
        close(myPipe);
        exit(0);
    }
}

void handleWritePipeError(int sig){
    if(sig == SIGPIPE){
        printf("parcheggio in corso...\n");
    }
}

int isStringValid(char string[], char *validStrings[]){
    for(int i = 0; validStrings[i] != NULL; i++){
        if(strcmp(string, validStrings[i]) == 0) return 1;
    } return 0;
}

void setMessage(struct message *msg, char type[], char data[]){
    if(msg == NULL) return;
    strcpy(msg->type, type);
    strcpy(msg->data, data);
}

int main(){

    signal(SIGUSR1, handleCloseInput);
    signal(SIGPIPE, handleWritePipeError);

    char input[10];
    struct message msg;
    char *validStrings[] = {"INIZIO", "PARCHEGGIO", "ARRESTO", NULL};

    myPipe = open("./pipe/sensorsPipe", O_WRONLY);
    if(myPipe == -1){
        perror("hmi_input");
        exit(-1);
    }

    while(1){
        printf(": ");
        scanf("%s", input);
        if(isStringValid(input, validStrings)){
            setMessage(&msg, input, "");
            write(myPipe, &msg, sizeof(msg));
        }
    }
}