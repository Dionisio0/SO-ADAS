#include <stdio.h>      // printf()
#include <unistd.h>     // read
#include <fcntl.h>      // open(), O_RDONLY
#include <string.h>     // strcpy()
#include "spl.h"

struct message {
    char type[13];
    char data[8];
};

int main(int argc, char *argv[]){
    struct sourcePipeLog spl = openSPL(argv[1], argv[2], argv[3]);

    unsigned char byte[8];
    char temp[5];
    struct message msg;
    int len;

    while(len = read(spl.source, byte, sizeof(byte))){
        if(len == 8){
            strcpy(msg.type, "BYTE");
            for(int i = 0; i < 4; i++){
                sprintf(temp, "%X%X", byte[i*2], byte[(i*2)+1]);
                strcpy(msg.data, temp);

                //pipe
                write(spl.pipe, &msg, sizeof(msg));

                //log
                write(spl.log, temp, strlen(temp));
                write(spl.log, "\n", 1);
            } 
        }
        sleep(1);
    }

    closeSPL(spl);

    return 0;
}