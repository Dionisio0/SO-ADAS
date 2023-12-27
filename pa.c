#include <stdio.h>      // printf()
#include <unistd.h>     // read
#include <fcntl.h>      // open(), O_RDONLY
#include <string.h>     // strcpy()
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include "spl.h"

struct message {
    char type[13];
    char data[8];
};

int main(int argc, char *argv[]){
    struct sourcePipeLog spl = openSPL(argv[1], argv[2], argv[3]);

    int svcPipe = open("./pipe/svcPipe", O_RDONLY | O_NONBLOCK);


    unsigned char byte[8];
    char temp[5];
    struct message msg;

    time_t startTime, currentTime;
    time(&startTime);
    time(&currentTime);
    
    int timer = 1;

    int randNum;
    srand(time(NULL));

    // ttl = durata del processo pa
    int ttl = 5;

    strcpy(msg.type, "BYTE");

    while(difftime(currentTime, startTime) < ttl){

        //tra svc e pa si può usare un altra struttura diversa da msg
        if(read(svcPipe, byte, sizeof(byte)) == 8){
            printf("ricevuto 4 hex da svc\n");
            for(int i = 0; i < 4; i++){
                sprintf(temp, "%X%X", byte[i*2], byte[(i*2)+1]);

                //pipe
                strcpy(msg.data, temp);
                write(spl.pipe, &msg, sizeof(msg));
               
            }
        }

        if(read(spl.source, byte, sizeof(byte)) == 8){
            printf("letto 8 byte da dev/urandom\n");
            for(int i = 0; i < 4; i++){
                sprintf(temp, "%X%X", byte[i*2], byte[(i*2)+1]);

                //pipe
                strcpy(msg.data, temp);
                write(spl.pipe, &msg, sizeof(msg));
               
                //log
                write(spl.log, temp, strlen(temp));
                write(spl.log, "\n", 1);
            } 
        }


        // per inviare in modo randomico un errore
        /*randNum = (rand() % 5);
        if(randNum == 2){
            strcpy(msg.type, "BYTE");
            strcpy(msg.data, "FAEE");
            write(spl.pipe, &msg, sizeof(msg));
        }*/
        
        sleep(1);
        time(&currentTime);
        // ttl = durata del processo pa
    }
    close(svcPipe);
    closeSPL(spl);

    return 0;
}