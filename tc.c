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

void setSpeed(){
    int shmid = shmget(123, sizeof(int), 0666);
    speed = (int *)shmat(shmid, NULL, 0);
}

int randomThrottleControlError(){
    int error = 100000;
    int randomNumber = (rand() % error);
    if(randomNumber == 0) return 1;
    else return 0;
}


int main(){

    signal(SIGUSR1, terminateHandler);

    char msg[13];
    char logMsg[36];
    time_t t;

    myPipe = open("./pipe/throttlePipe", O_RDONLY);
    if(myPipe == -1){
        perror("throttle control");
        exit(-1);
    }

    myLog = open("./log/throttle.log", O_WRONLY | O_TRUNC);
    if(myLog == -1){
        perror("throttle control");
        exit(-1);
    }

    srand(time(NULL));
    setSpeed();

    while(1){
        read(myPipe, msg, sizeof(msg));
        if(strcmp(msg, "INCREMENTO 5") == 0){

            if(randomThrottleControlError()){
                kill(getppid(), SIGUSR1);
            } else {
                time(&t);
                sprintf(logMsg, "%sAUMENTO 5\n", ctime(&t));
                logMsg[24] = ' '; // per sostituire ' ' con '\n' che viene aggiunto da ctime()

                write(myLog, logMsg, strlen(logMsg));

                *speed += 5;

                // da togliere la seguente riga
                printf("VELOCITAAAAAAAA: %d\n", *speed);
            }
        }
    }

    return 0;
}