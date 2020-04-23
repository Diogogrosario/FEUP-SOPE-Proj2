#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define MAX_THREADS 100

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

void printMessage(message *msg){
    printf("i: %d\n", msg->i);
    printf("pid: %d\n", msg->pid);
    printf("tid: %ld\n", msg->tid);
    printf("dur: %d\n", msg->dur);
    printf("pl: %d\n", msg->pl);
}

void * receiveRequest(void * msg) {
    message request = *(message*)msg;
    printMessage(&request);

    char* aux = malloc(sizeof(char)*100);
    sprintf(aux,"/tmp/%d.%ld", request.pid, request.tid);
    mkfifo(aux, 0666);
    
    int fd = open(aux, O_WRONLY);
    message *toSend = malloc(sizeof(message));
    *toSend = makeMessage(request.i, request.dur, request.i);
    write(fd, toSend, sizeof(message));

    return NULL;
}

int main(int argc, char *argv[]) {
    flags flags;

    pthread_t threads[MAX_THREADS];

    initFlags(&flags);
    setFlags(argc, argv, &flags);

    char* aux = malloc(sizeof(char)*100);
    sprintf(aux,"/tmp/%s", flags.fifoname);
    mkfifo(aux, 0666);
    
    int fd = open(aux, O_RDONLY);
    message *toReceive = malloc(sizeof(message));
    

    time_t start = time(NULL);

    
    while(time(NULL)-start < flags.nsecs){
        
        if(read(fd, toReceive, sizeof(message))> 0) {
            
            pthread_create(&threads[toReceive->i], NULL, receiveRequest, toReceive);
        }
    }

    close(fd);
    unlink(aux);

    return 0;
}