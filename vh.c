#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

int main(){
    
    int sp = open("./pipe/speedPipe", O_RDONLY | O_NONBLOCK);
    int throttlePipe = open("./pipe/throttlePipe", O_WRONLY);
    int brakePipe = open("./pipe/brakePipe", O_WRONLY);


    int hmiOuputPipe = open("./pipe/hmiOutputPipe", O_WRONLY);
    int ecuLog = open("./log/ecu.log", O_WRONLY | O_APPEND);

    int process = 0;

    // per salvare la velocità letta prima
    int targetSpeed = 0;

    int speedMsg;

    char trMsg[13] = "INCREMENTO 5";
    char bwMsg[13] = "DECREMENTO 5";

    //key_t key = ftok("memfile", 'P');
    int shmid = shmget(123, sizeof(int), 0666);
    int *currentSpeed = (int *)shmat(shmid, NULL, 0);

    //provare con sem_open("my_semaphore", 0);
    //sem_t *semaphore = sem_open("my_semaphore", O_CREAT | O_EXCL, 0666, 1);

    //int currentSpeed;

    while(1){

        // è necessario una semaforo qui ???
        // se dopo la write() aspetto 1sec sono sicuro
        // che il dato in memoria condivisa sia già
        // stato aggiornato
        /*sem_wait(semaphore);
        currentSpeed = *speed;
        sem_post(semaphore);*/

        if(read(sp, &speedMsg, sizeof(speedMsg)) == sizeof(speedMsg)) {
            targetSpeed = speedMsg;
        }

        if(targetSpeed > *currentSpeed){
            write(throttlePipe, trMsg, 13);

            //hmi_output
            write(hmiOuputPipe, trMsg, 13);

            //log
            write(ecuLog, trMsg, 12);
            write(ecuLog, "\n", 1);

            sleep(1);
        } else if(targetSpeed < *currentSpeed){
            write(brakePipe, bwMsg, 13);

            //hmi_output
            write(hmiOuputPipe, bwMsg, 13);

            //log
            write(ecuLog, bwMsg, 12);
            write(ecuLog, "\n", 1);

            sleep(1);
        }   
    }
    //sem_close(semaphore);
    shmdt(currentSpeed);
    close(hmiOuputPipe);
    close(ecuLog);
    close(sp);
    close(throttlePipe);
    close(brakePipe);


    return 0;
}