#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#define MAX_THREADS 100

typedef struct{
    int nsecs;
    char* fifoname;
} flags;

typedef struct{
    int i;
    pid_t pid;
    pthread_t tid;
    int dur;
    int pl;
} message;

int fd;

void printFlags(flags * flags){
    printf("nsecs: %d\n", flags->nsecs);
    printf("fifoname: %s\n", flags->fifoname);
}

void initFlags(flags *flags){
   flags->nsecs = 0;
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

void *sendRequest(void * thread_no) {
    message msg = makeMessage(*(int*)thread_no, 100, -1);

    write(fd, &msg, sizeof(msg));

    char* aux = malloc(sizeof(char)*100);
    sprintf(aux,"/tmp/%d.%ld", msg.pid, msg.tid);
    
    sleep(1);
    int fdRequest;
    if((fdRequest = open(aux, O_RDONLY)) == -1) {
        printf("Error opening message from the server!\n");
        exit(1);
    }
    message *toReceive = malloc(sizeof(message));
    read(fdRequest, toReceive, sizeof(message));

    printf("Received from server\n");
    printMessage(toReceive);
    unlink(aux);

    return NULL;
}

int main(int argc, char *argv[]) {
    flags flags; 
    
    pthread_t threads[MAX_THREADS];

    initFlags(&flags);
    setFlags(argc, argv, &flags);


    char* aux = malloc(sizeof(char)*100);
    sprintf(aux, "/tmp/%s", flags.fifoname);
    
    if((fd = open(aux, O_WRONLY)) == -1){
        printf("No server fifo, can't make request\n");
        exit(1);
    }

    int thread_no = 0;
    time_t start = time(NULL);
    
    while(time(NULL)-start < flags.nsecs){
        //printf("%ld\n", time(NULL)-start);
        thread_no += 1;
        pthread_create(&threads[thread_no], NULL, sendRequest, &thread_no);
        usleep(100000);
    }

    for(int i = 0; i <= thread_no; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}