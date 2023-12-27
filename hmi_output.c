#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

int myPipe;

void terminateHandler(int sig){
    if(sig == SIGUSR1){
        close(myPipe);
        exit(0);
    }
}

int main(){

    signal(SIGUSR1, terminateHandler);

    // "INCREMENTO 5" lunghezza = 11 ? (+1 per '/0')??
    char msg[13];

    myPipe = open("./pipe/hmiOutputPipe", O_RDONLY);
    if(myPipe == -1){
        perror("hmi_output");
        exit(-1);
    }

    while(1){
        //aggiungere controllo per la read ? 
        // è proprio necessario ???
        read(myPipe, &msg, sizeof(msg));
        printf("hmi output: %s\n", msg);
    }

    return 0;
}