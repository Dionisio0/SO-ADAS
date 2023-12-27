#include <stdio.h>      // printf(), perror()
#include <unistd.h>     // fork(), execl(), read()
#include <sys/stat.h>   // S_IFIFO
#include <fcntl.h>      // O_RDONLY
#include <string.h>     // strcmp()
#include <stdlib.h>     // atoi()
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>


char *source;

struct components{
	int output;
    int fwc;
    int ffr;
    int vh;
    int tc;
    int bw;
    int sw;
    int pa;
    int svc;
};

struct pipes{
	int output;
    int sensors;
    int steer;
    int throttle;
    int brake;
    int speed;
};

struct message {
    char type[13];
    char data[8];
};

void initSpeed(){
    //key_t key = ftok("memfile", 'P');
    int shmid = shmget(123, sizeof(int), IPC_CREAT | 0666);
    int *speed = (int *)shmat(shmid, NULL, 0);
    *speed = 0;
    shmdt(speed);
}

void createNamedPipe(char *name){
    unlink(name);
    mknod(name, S_IFIFO, 0);
    chmod(name, 0660);
}

void createPipes(/*accettare una lista variabile di stringhe in modo da poter creare altrettante pipe*/){

	// pipe per l'output
	createNamedPipe("./pipe/hmiOutputPipe");

    // pipe per i sensori
    createNamedPipe("./pipe/sensorsPipe");

    // pipe per throttle control
    createNamedPipe("./pipe/throttlePipe");

    // pipe per brake-by-wire
    createNamedPipe("./pipe/brakePipe");

    // pipe per steer-by-wire
    createNamedPipe("./pipe/steerPipe");

    // pipe per il controllo della velocità
    createNamedPipe("./pipe/speedPipe");
}

struct components comp;
void startComponents(){
	if((comp.output = fork()) == 0){execl("./hmi_output", "hmi_output", NULL);}
    if((comp.fwc = fork()) == 0){execl("./fwc", "fwc", "frontCamera.data", "./pipe/sensorsPipe", "./log/camera.log",NULL);}
    if((comp.ffr = fork()) == 0){execl("./ffr", "ffr", source, "./pipe/sensorsPipe","./log/radar.log", NULL);}
    if((comp.tc = fork()) == 0){execl("./tc", "tc", NULL);}
    if((comp.bw = fork()) == 0){execl("./bw", "bw", NULL);}
    if((comp.sw = fork()) == 0){execl("./sw", "sw", NULL);}
    if((comp.vh = fork()) == 0){execl("./vh", "vh", NULL);}
}
void killSensors(){
    kill(comp.fwc, SIGTERM);
    kill(comp.ffr, SIGTERM);
}
void killActuators(){
    kill(comp.vh, SIGTERM);
    kill(comp.tc, SIGUSR1);
    kill(comp.bw, SIGUSR1);
    kill(comp.sw, SIGUSR1);
}


struct pipes pipes;
void openPipes(){
	
    pipes.sensors = open("./pipe/sensorsPipe", O_RDONLY);
    pipes.steer = open("./pipe/steerPipe", O_WRONLY);
    pipes.speed = open("./pipe/speedPipe", O_WRONLY);
	pipes.output = open("./pipe/hmiOutputPipe", O_WRONLY);
}
void closePipes(){
	close(pipes.output);
    close(pipes.steer);
    close(pipes.sensors);
    close(pipes.speed);
}


void handleThrottleControlError(int sig){
    if(sig == SIGUSR1){
        printf("segnale di errore accelerazione\n");
        kill(comp.bw, SIGUSR1);
    }
}

void reduceSpeedToZero(){
    int shmid = shmget(123, sizeof(int), IPC_CREAT | 0666);
    int *speed = (int *)shmat(shmid, NULL, 0);

    int targetSpeed = 0;
    write(pipes.speed, &targetSpeed, sizeof(targetSpeed));

    while(*speed != 0){sleep(1);}

    shmdt(speed);
}

void startParking(){
    printf("avvio procedura di parcheggio...\n");

    close(pipes.sensors);

    createNamedPipe("./pipe/sensorsPipe");
    createNamedPipe("./pipe/svcPipe");


    if((comp.pa = fork()) == 0){execl("./pa", "pa", source, "./pipe/sensorsPipe", "./log/assist.log", NULL);}
    if((comp.svc = fork()) == 0){execl("./svc", "svc", source, "./pipe/svcPipe", "./log/cameras.log", NULL);}

    pipes.sensors = open("./pipe/sensorsPipe", O_RDONLY);
}

int invalidData(struct message msg/*cambiare con char *str*/){
    if(strcmp(msg.data, "172A") == 0 || strcmp(msg.data, "D693") == 0 ||
        strcmp(msg.data, "") == 0 || strcmp(msg.data, "BDD8") == 0 ||
        strcmp(msg.data, "FAEE") == 0 || strcmp(msg.data, "4300") == 0){
        return 1;
    } return 0;
}
void killParkingProcess(){
    kill(comp.pa, SIGTERM);
    waitpid(comp.pa, NULL, 0);
    kill(comp.svc, SIGTERM);
    waitpid(comp.svc, NULL, 0);
}

void waitStartCommand(){
    struct message msg;
    do{
        read(pipes.sensors, &msg, sizeof(msg));
    }while(strcmp(msg.type, "INIZIO") != 0);
}

enum action{
    BYTE,
    NUMERO,
    DIREZIONE,
    PARCHEGGIO,
    ERRORE
};
enum action getAction(char *str){
    if(strcmp(str, "BYTE") == 0){return BYTE;}
    else if(strcmp(str, "NUMERO") == 0){return NUMERO;}
    else if(strcmp(str, "DESTRA") == 0 || strcmp(str, "SINISTRA") == 0){return DIREZIONE;}
    else if(strcmp(str, "PARCHEGGIO") == 0){return PARCHEGGIO;}
    else if(strcmp(str, "PERICOLO") == 0 || strcmp(str, "ARRESTO") == 0){return ERRORE;}
    return -1;
}
void init(){
    signal(SIGUSR1, handleThrottleControlError);
    initSpeed();
    createPipes();
    startComponents();
    openPipes();
    waitStartCommand();
}


int findProcessPID(char *processName){
    FILE *fp;
    char command[50];
    int pid;

    sprintf(command, "pgrep %s", processName);

    fp = popen(command, "r");
    fscanf(fp, "%d", &pid);

    pclose(fp);

    return pid;
}


int main(int argc, char *argv[]){

    if(argc > 1){
        if(strcmp(argv[1], "NORMALE") == 0){source = "/dev/urandom";} 
        else if(strcmp(argv[1], "ARTIFICIALE") == 0){source = "urandomARTIFICIALE.binary";}
        else {return 1;}
    } else {return 1;}

    printf("avvio modalità: %s\n", source);
    

    init();

	int log = open("./log/ecu.log", O_WRONLY | O_TRUNC | O_APPEND);

    struct message msg;
    int readDataParking = 0;

	

    while(read(pipes.sensors, &msg, sizeof(msg)) /*readMsg(msg)*/){
        
        printf("info: %s\ndata: %s\n\n", msg.type, msg.data);

        //handleMsg(msg);

        switch(getAction(msg.type)){

            case BYTE: 
                if(readDataParking && invalidData(msg)){
                    killParkingProcess();
                    startParking();
                }
                break;

            case NUMERO:
                int targetSpeed = atoi(msg.data);
                write(pipes.speed, &targetSpeed, sizeof(targetSpeed));
                break;

            case DIREZIONE:
				//hmi_output
				write(pipes.output, msg.type, sizeof(msg.type));

				//log
				write(log, msg.type, strlen(msg.type));
				write(log, "\n", 1);

                //write(outputEcuPipe, &msg, sizeof(msg));

				//command
                write(pipes.steer, msg.type, sizeof(msg.type));
                break;

            case PARCHEGGIO:
                killSensors();
                reduceSpeedToZero();
                killActuators();
                startParking();
                readDataParking = 1;
                break;

            case ERRORE:
				//hmi_output
				write(pipes.output, msg.type, sizeof(msg.type));

				//log
				write(log, msg.type, strlen(msg.type));
				write(log, "\n", 1);

                //write(outputEcuPipe, &msg, sizeof(msg));

                kill(comp.bw, SIGUSR2);
                break;
        }

        //write ecu.log
        //write hmi_output
    }
    // devo terminare hmi_output ???
    kill(comp.output, SIGUSR1);
	close(log);
    
	//terminare il processo hmi_output
    closePipes();

    while(wait(NULL) > 0){}

    // RIMANE DA TERMINARE IL PROCESSO HMI_INPUT
    kill(findProcessPID("hmi_input"), SIGUSR1);
    printf("corsa terminata\n");
    
    return 0;
}


