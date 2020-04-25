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

#define MAX_THREADS 500

struct timespec start;

typedef struct{
    int nsecs;
    char* fifoname;
} flagsList;


flagsList flags; 

typedef struct{
    int i;
    pid_t pid;
    pthread_t tid;
    int dur;
    int pl;
} message;

int fd;

void printFlags(flagsList * flags){
    printf("nsecs: %d\n", flags->nsecs);
    printf("fifoname: %s\n", flags->fifoname);
}

void initFlags(flagsList *flags){
   flags->nsecs = 0;
}

void setFlags(int argc, char* argv[], flagsList *flags){
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

message makeMessage(int i, int pl){
    message msg;
    msg.i = i;
    msg.pid = getpid();
    msg.tid = pthread_self();
    msg.dur = (rand()%99)+1;
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

void *sendRequest(void *thread_no)
{
    message msg = makeMessage(*(int *)thread_no, -1);

    char* publicFIFO = malloc(sizeof(char)*100);
    sprintf(publicFIFO, "/tmp/%s", flags.fifoname);
    
    

    printMessage(&msg, "IWANT");

    if((fd = open(publicFIFO, O_WRONLY)) == -1){
        printMessage(&msg,"CLOSD");
        return NULL;
    }

    write(fd, &msg, sizeof(msg));

    char *aux = malloc(sizeof(char) * 100);
    sprintf(aux, "/tmp/%d.%ld", msg.pid, msg.tid);

    mkfifo(aux, 0666);
    int fdRequest;

    if ((fdRequest = open(aux, O_RDONLY)) == -1)
    {
        return NULL;
    }

    message *toReceive = malloc(sizeof(message));
    int nTries = 0;
    while (read(fdRequest, toReceive, sizeof(message)) <= 0 && nTries < 8)
    {
        nTries++;
        usleep(1000);
    }
    if(nTries>=8)
    {
        printMessage(&msg,"FAILD");
        return NULL;
    }
    printMessage(toReceive, "IAMIN");
    close(fdRequest);

    unlink(aux);

    return NULL;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    pthread_t threads[MAX_THREADS];

    initFlags(&flags);
    setFlags(argc, argv, &flags);

    char* publicFIFO = malloc(sizeof(char)*100);
    sprintf(publicFIFO, "/tmp/%s", flags.fifoname);
    

    if((fd = open(publicFIFO, O_WRONLY)) == -1){
    }

   

    int thread_no = 0;
    clock_gettime(CLOCK_REALTIME,&start);

    struct timespec timeNow;
    clock_gettime(CLOCK_REALTIME,&timeNow);
    while(timeNow.tv_sec-start.tv_sec < flags.nsecs){
        if (thread_no > MAX_THREADS)
        {
            break;
        }
        
        pthread_create(&threads[thread_no], NULL, sendRequest, &thread_no);
        thread_no += 1;
        usleep((rand()%50000)+50000);
        clock_gettime(CLOCK_REALTIME,&timeNow);
    }

    for(int i = 0; i < thread_no; i++) {
        pthread_join(threads[i], NULL);
    }

    close(fd);  

    return 0;
}