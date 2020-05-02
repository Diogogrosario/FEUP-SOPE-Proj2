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
#include <semaphore.h>
#include <stdbool.h>

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

bool closed = false;


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
    msg.dur = (rand()%800)+200;
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

    
    char* semName = malloc(sizeof(char)*255);
    sprintf(semName, "sem.%d.%ld", msg.pid, msg.tid);
    sem_t *sem_id = sem_open(semName, O_CREAT, 0600, 0);

    char *semName2 = malloc(sizeof(char) * 255);
    sprintf(semName2, "sem2.%d.%ld", msg.pid, msg.tid);
    sem_t *sem_id2 = sem_open(semName2, O_CREAT, 0600, 0);

    if(sem_id2 == NULL) {
        fprintf(stderr, "Error on sem_open for %s\n", semName2);
        return NULL;
    }
    if(sem_id == NULL) {
        fprintf(stderr, "Error on sem_open for %s\n", semName);
        return NULL;
    }

    if((fd = open(publicFIFO, O_WRONLY|O_NONBLOCK)) == -1){
        printMessage(&msg,"CLOSD");
        closed = true;
        return NULL;
    }


    if(write(fd, &msg, sizeof(msg)) == -1) {
        perror("Error on writing client request to public FIFO\n");
        return NULL;
    }
    

    char *aux = malloc(sizeof(char) * 100);
    sprintf(aux, "/tmp/%d.%ld", msg.pid, msg.tid);

    
    if(mkfifo(aux, 0666) == -1) {
        perror("Error on making private FIFO\n");
        return NULL;
    }
    //UNLOCK SERVER CUZ FIFO IS CREATED

    int fdRequest;

    

    if(sem_post(sem_id)<0){
        fprintf(stderr, "Error on sem_post for %s\n", semName);
        return NULL;
    }
    
    if ((fdRequest = open(aux, O_RDONLY)) == -1)
    {
        perror(aux);
        return NULL;
    }

    message *toReceive = malloc(sizeof(message));

    //WAITING FOR SERVER RESPONSE
    if(sem_wait(sem_id2)<0){
        fprintf(stderr, "Error on sem_wait for %s\n", semName2);
        return NULL;
    }
    if(read(fdRequest, toReceive, sizeof(message)) <= 0)
    {
        printMessage(&msg,"FAILD");
        return NULL;
    }

    if(toReceive->pl == -1) {
        printMessage(toReceive, "CLOSD");
        closed = true;
    }
    else {
        printMessage(toReceive, "IAMIN");
    }

    close(fdRequest);

    unlink(aux);
    free(semName);
    free(semName2);

    return NULL;
}

int main(int argc, char *argv[]) {

    srand(time(NULL));
    
    pthread_t threads[MAX_THREADS];

    initFlags(&flags);
    setFlags(argc, argv, &flags);

    int thread_no = 0;
    clock_gettime(CLOCK_REALTIME,&start);

    struct timespec timeNow;
    clock_gettime(CLOCK_REALTIME,&timeNow);
    while(timeNow.tv_sec-start.tv_sec < flags.nsecs && !closed){
        if (thread_no > MAX_THREADS)
        {
            break;
        }
        
        if(pthread_create(&threads[thread_no], NULL, sendRequest, &thread_no) == 0) {
            thread_no += 1;
            usleep((rand()%50000)+50000); //time between requests
        }
        else {
            fprintf(stderr, "Error on creating thread %d on client\n", thread_no);
        }

        clock_gettime(CLOCK_REALTIME,&timeNow);
    }

    for(int i = 0; i < thread_no; i++) {
        pthread_join(threads[i], NULL);
    }

    close(fd);  

    return 0;
}