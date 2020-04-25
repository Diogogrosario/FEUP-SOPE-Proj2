#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <pthread.h>

#define MAX_THREADS 100

struct timespec start;
bool finished = false;

typedef struct{
    int nsecs;
    int nplaces;
    int nthreads;
    char* fifoname;
} flags;

typedef struct{
    int i;
    pid_t pid;
    pthread_t tid;
    int dur;
    int pl;
} message;

void printFlags(flags *flags){
    printf("nsecs: %d\n", flags->nsecs);
    printf("nplaces: %d\n", flags->nplaces);
    printf("nthreads: %d\n", flags->nthreads);
    printf("fifoname: %s\n", flags->fifoname);
}

void initFlags(flags *flags){
   flags->nsecs = 0;
   flags->nplaces = 0;
   flags->nthreads = 0;

}

void setFlags(int argc, char* argv[], flags *flags){
    for(int i = 1; i < argc; i++){
        if (!strcmp(argv[i], "-t")){
            i++;
            for (int j = 0; j < strlen(argv[i]); ++j)
            {
                if (argv[i][j] > '9' || argv[i][j] < '0')
                {
                    printf("Invalid time!\n");
                    exit(1);
                }
            }
            flags->nsecs = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-l")){
            i++;
            for (int j = 0; j < strlen(argv[i]); ++j)
            {
                if (argv[i][j] > '9' || argv[i][j] < '0')
                {
                    printf("Invalid capacity!\n");
                    exit(1);
                }
            }
            flags->nplaces = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-n")){
            i++;
            for (int j = 0; j < strlen(argv[i]); ++j)
            {
                if (argv[i][j] > '9' || argv[i][j] < '0')
                {
                    printf("Invalid number of threads!\n");
                    exit(1);
                }
            }
            flags->nthreads = atoi(argv[i]);
        }
        else{
            flags->fifoname = argv[i];
        }
    }
}

message makeMessage(int i, int dur, int pl){
    message msg;
    msg.i = i;
    msg.pid = getpid();
    msg.tid = pthread_self();
    msg.dur = dur;
    msg.pl = pl;
    return msg;
}

void printMessage(message *msg, char* op){
    printf("%ld; ",time(NULL));
    printf("%d; ", msg->i);
    printf("%d; ", msg->pid);
    printf("%ld; ", msg->tid);
    printf("%d; ", msg->dur);
    printf("%d; ", msg->pl);
    printf("%s\n", op);
}

void * receiveRequest(void * msg) {
    message request = *(message*)msg;
    printMessage(&request, "RECVD");
    

    char* aux = malloc(sizeof(char)*100);
    sprintf(aux, "/tmp/%d.%ld", request.pid, request.tid);

    int nTries = 0;
    int fdSend;
    while ((fdSend = open(aux, O_WRONLY)) == -1 && nTries < 4)
    {
        nTries++;
        usleep(1000);
    }
    if (fdSend == -1)
    {
        printMessage(&request,"GAVUP");
        return NULL;
    }
    message *toSend = malloc(sizeof(message));
    *toSend = makeMessage(request.i, request.dur, request.i);
    write(fdSend, toSend, sizeof(message));

    printMessage(toSend, "ENTER");

    struct timespec durLeft;
    clock_gettime(CLOCK_REALTIME,&durLeft);
    while(durLeft.tv_nsec/1000000.0<request.dur)
    {
        if(finished)
        {
            printMessage(toSend,"2LATE");
            close(fdSend);

            free(aux);
            free(toSend);

            return NULL;
        }
        clock_gettime(CLOCK_REALTIME,&durLeft);
    }
    printMessage(toSend, "TIMUP");
    close(fdSend);

    free(toSend);
    free(aux);

    return NULL;
}

int main(int argc, char *argv[]) {
    flags flags;
    pthread_t threads[MAX_THREADS];
    int nThreads=0;

    initFlags(&flags);
    setFlags(argc, argv, &flags);

    char* publicFIFO = malloc(sizeof(char)*100);
    sprintf(publicFIFO,"/tmp/%s", flags.fifoname);
    mkfifo(publicFIFO, 0666);
    
    int fd = open(publicFIFO, O_RDONLY);
    message *toReceive = malloc(sizeof(message));
    

    clock_gettime(CLOCK_REALTIME,&start);

    struct timespec timeNow;
    clock_gettime(CLOCK_REALTIME,&timeNow);
    while(timeNow.tv_sec-start.tv_sec < flags.nsecs){
        if(nThreads >= MAX_THREADS){
            printf("Can't create more threads!\n");
            break;
        }
        else if(read(fd, toReceive, sizeof(message))> 0) {     
            pthread_create(&threads[nThreads], NULL, receiveRequest, toReceive);
            nThreads++;
        }
        
        clock_gettime(CLOCK_REALTIME,&timeNow);
    }

    close(fd);
    unlink(publicFIFO);
    finished = true;

    for(int i=0;i<nThreads;i++)
    {
        pthread_join(threads[i], NULL);  
    }


    
    free(publicFIFO);
    free(toReceive);
    return 0;
}