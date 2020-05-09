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
#include <semaphore.h>
#include <signal.h>
#include "queue.h"

#define MAX_THREADS 500000

struct timespec start;
bool finished = false;
bool hasFifoName = false;
int nThreads = 0;
int nOccupied = 0;
sem_t *sem_id_threads;

pthread_mutex_t printMut = PTHREAD_MUTEX_INITIALIZER;

bool threadSem;

typedef struct
{
    int nsecs;
    int nplaces;
    int nthreads;
    char *fifoname;
} flag;

flag flags;

typedef struct
{
    int i;
    pid_t pid;
    pthread_t tid;
    int dur;
    int pl;
} message;

void printFlags()
{
    printf("nsecs: %d\n", flags.nsecs);
    printf("nplaces: %d\n", flags.nplaces);
    printf("nthreads: %d\n", flags.nthreads);
    printf("fifoname: %s\n", flags.fifoname);
}

void initFlags()
{
    flags.nsecs = 0;
    flags.nplaces = 0;
    flags.nthreads = 0;
}

void setFlags(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-t"))
        {
            i++;
            for (int j = 0; j < strlen(argv[i]); ++j)
            {
                if (argv[i][j] > '9' || argv[i][j] < '0')
                {
                    printf("Invalid time!\n");
                    exit(1);
                }
            }
            flags.nsecs = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-l"))
        {
            i++;
            for (int j = 0; j < strlen(argv[i]); ++j)
            {
                if (argv[i][j] > '9' || argv[i][j] < '0')
                {
                    printf("Invalid capacity!\n");
                    exit(1);
                }
            }
            flags.nplaces = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-n"))
        {
            i++;
            for (int j = 0; j < strlen(argv[i]); ++j)
            {
                if (argv[i][j] > '9' || argv[i][j] < '0')
                {
                    printf("Invalid number of threads!\n");
                    exit(1);
                }
            }
            flags.nthreads = atoi(argv[i]);
        }
        else
        {
            flags.fifoname = argv[i];
            hasFifoName = true;
        }
    }
}

message makeMessage(int i, int dur, int pl)
{
    message msg;
    msg.i = i;
    msg.pid = getpid();
    msg.tid = pthread_self();
    msg.dur = dur;
    msg.pl = pl;
    return msg;
}

void printMessage(message *msg, char *op)
{
    pthread_mutex_lock(&printMut);
    printf("%ld; ", time(NULL));
    printf("%d; ", msg->i);
    printf("%d; ", msg->pid);
    printf("%ld; ", msg->tid);
    printf("%d; ", msg->dur);
    printf("%d; ", msg->pl);
    printf("%s\n", op);
    pthread_mutex_unlock(&printMut);
}

void sig_handler(int intType)
{
    switch (intType)
    {
    case SIGALRM:
        finished = true;
        break;
    default:
        break;
    }
}

void *receiveRequest(void *msg)
{
    pthread_detach(pthread_self());
    message request = *(message *)msg;
    //free(msg);
    printMessage(&request, "RECVD");

    //SEMAPHORE
    char *semName = malloc(sizeof(char) * 255);
    sprintf(semName, "sem.%d.%ld", request.pid, request.tid);
    sem_t *sem_id = sem_open(semName, O_CREAT, 0600, 0);

    char *aux = malloc(sizeof(char) * 100);
    sprintf(aux, "/tmp/%d.%ld", request.pid, request.tid);

    if (sem_id == NULL)
    {
        fprintf(stderr, "Error on sem_open for %s\n", semName);
        if (threadSem)
            sem_post(sem_id_threads);
        return NULL;
    }

    int fdSend;

    //LOCK, WAITING FOR CLIENT TO OPEN FIFO
    if (sem_wait(sem_id) < 0)
    {
        fprintf(stderr, "Error on sem_wait for %s\n", semName);
        if (threadSem)
            sem_post(sem_id_threads);
        return NULL;
    }

    //ABRIR FIFO PRIVADO
    if ((fdSend = open(aux, O_WRONLY)) == -1)
    {
        printMessage(&request, "GAVUP");
        if (threadSem)
            sem_post(sem_id_threads);
        return NULL;
    }

    message *toSend = malloc(sizeof(message));

    if (!finished)
    {
        *toSend = makeMessage(request.i, request.dur, request.i);
    }
    else
    {
        *toSend = makeMessage(request.i, request.dur, -1);
    }

    if (write(fdSend, toSend, sizeof(message)) == -1)
    {
        perror("Error on writing server response to public FIFO\n");
    }

    if (finished)
    {
        printMessage(toSend, "2LATE");
        close(fdSend);
        free(aux);
        free(toSend);
        free(semName);
        if (threadSem)
            sem_post(sem_id_threads);
        return NULL;
    }

    printMessage(toSend, "ENTER");

    usleep(request.dur * 1000);

    printMessage(toSend, "TIMUP");

    close(fdSend);

    free(toSend);
    free(aux);
    free(semName);

    if (threadSem)
        sem_post(sem_id_threads);
    return NULL;
}

int main(int argc, char *argv[])
{

    pthread_t thread;

    struct sigaction action;
    action.sa_handler = sig_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    initFlags();
    setFlags(argc, argv);

    if (flags.nsecs == 0 || !hasFifoName)
    {
        fprintf(stderr, "Correct usage: Q1 -t n fifoName\n");
        pthread_exit(0);
    }

    nThreads = flags.nthreads;

    sigaction(SIGALRM, &action, NULL);

    alarm(flags.nsecs);

    char *semThreads = malloc(sizeof(char) * 255);
    sprintf(semThreads, "sem4");
    sem_id_threads = sem_open(semThreads, O_CREAT, 0600, nThreads);
    if (sem_id_threads == NULL)
    {
        fprintf(stderr, "Error on sem_open for %s\n", semThreads);
        return 1;
    }
    threadSem = true;

    char *publicFIFO = malloc(sizeof(char) * 100);
    sprintf(publicFIFO, "/tmp/%s", flags.fifoname);
    mkfifo(publicFIFO, 0666);

    int fd = open(publicFIFO, O_RDONLY);
    if (fd == -1 && errno == EINTR)
    {
        perror("Could not open public FIFO for reading");
        finished = true;
    }
    message *toReceive = malloc(sizeof(message));

    int nClients = 0;
    while (nClients != 0 || !finished)
    {
        if ((nClients = read(fd, toReceive, sizeof(message))) > 0)
        {

            sem_wait(sem_id_threads);

            if (pthread_create(&thread, NULL, receiveRequest, toReceive) != 0)
            {
                fprintf(stderr, "Error on creating thread %d on server\n", nThreads);
            }
            toReceive = malloc(sizeof(message));
        }
    }
    finished = true;

    if (fd != -1)
        close(fd);
    unlink(publicFIFO);

    free(publicFIFO);

    threadSem = false;
    sem_close(sem_id_threads);
    sem_unlink(semThreads);
    free(semThreads);

    pthread_exit(0);
}