#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>

int myPipe;
int myLog;
int *speed;

void terminateHandler(int sig){
    if(sig == SIGUSR1){
        shmdt(speed);
        close(myPipe);
        close(myLog);
        exit(0);
    }
}

void stopHandler(int sig){
    if(sig == SIGUSR2){
        printf("gestisco il segnale di ARRESTO inviato da ECU \n");

        // scrive nel log "ARRESTO AUTO"
        write(myLog, "ARRESTO AUTO\n", 13);

        // imposta la velocità a 0
        *speed = 0;
    }
}

void setSpeed(){
    int shmid = shmget(123, sizeof(int), 0666);
    speed = (int *)shmat(shmid, NULL, 0);
}

int main(){
    signal(SIGUSR1, terminateHandler);
    signal(SIGUSR2, stopHandler);

    char msg[13];
    char logMsg[36];
    time_t t;

    myPipe = open("./pipe/brakePipe", O_RDONLY);
    if(myPipe == -1){
        perror("brake by wire");
        exit(-1);
    }

    myLog = open("./log/brake.log", O_WRONLY | O_TRUNC);
    if(myLog == -1){
        perror("brake by wire");
        exit(-1);
    }

    /*
    if(myPipe == -1 || myLog == -1){
        perror("brake by wire");
        exit(-1);
    }
    */

    setSpeed();

    while(1){
        read(myPipe, msg, sizeof(msg));
        if(strcmp(msg, "DECREMENTO 5") == 0){
            time(&t);
            sprintf(logMsg, "%sFRENO 5\n", ctime(&t));
            logMsg[24] = ' ';

            write(myLog, logMsg, strlen(logMsg));

            *speed -= 5;

            // da togliere la seguente riga
            printf("VELOCITAAAAAAAA: %d\n", *speed);
        }
    }

    return 0;
}