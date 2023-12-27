#include <stdio.h>      // printf() 
#include <unistd.h>     // read()
#include <fcntl.h>      // open(), O_RDONLY
#include <string.h>     // strcmp(), strcpy()
#include "spl.h"

int readLine(int fd, char *str){
    int n, i = 0;
    char c;

    do{
        n = read(fd, &c, sizeof(c));
        str[i++] = c;
    }while(n > 0 && c != '\n');

    str[--i] = '\0';
    return (n > 0);
}

int isStringValid(char string[], char *validStrings[]){
    for(int i = 0; validStrings[i] != NULL; i++){
        if(strcmp(string, validStrings[i]) == 0) return 1;
    } return 0;
}

struct message {
    char type[13];
    char data[8];
};

void setMessage(struct message *msg, char type[], char data[]){
    if(msg == NULL) return;
    strcpy(msg->type, type);
    strcpy(msg->data, data);
}




int main(int argc, char *argv[]){
    struct sourcePipeLog spl = openSPL(argv[1], argv[2], argv[3]);

    char str[11];
    char *validStrings[] = {"PARCHEGGIO", "DESTRA", "SINISTRA", "PERICOLO", NULL};
    struct message msg;

    while(readLine(spl.source, str) > 0){
        if(isStringValid(str, validStrings)){
            //setMessage(str, "");
            strcpy(msg.type, str);
            strcpy(msg.data, "");
        } else {
            //setMessage("NUMERO", str);
            strcpy(msg.type, "NUMERO");
            strcpy(msg.data, str);
        }

        //pipe
        write(spl.pipe, &msg, sizeof(msg));

        //log
        write(spl.log, str, strlen(str));
        write(spl.log, "\n", 1);

        sleep(1);
    }

    closeSPL(spl);

    return 0;
}