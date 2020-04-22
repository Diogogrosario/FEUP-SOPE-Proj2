#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

typedef struct{
    int nsecs;
    char* fifoname;
}flags;

typedef struct{
    int i;
    pid_t pid;
    pthread_t tid;
    int dur;
    int pl;
}message;

void printFlags(flags *flags){
    printf("nsecs: %d\n",flags->nsecs);
    printf("fifoname: %s\n",flags->fifoname);
}

void initFlags(flags *flags){
   flags->nsecs = 0;
}

void setFlags(int argc,char* argv[],flags *flags){
    for(int i = 1;i<argc;i++){
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

message makeMessage(int i,int dur,int pl){
    message msg;
    msg.i = i;
    msg.pid = getpid();
    msg.tid = pthread_self();
    msg.dur = dur;
    msg.pl = pl;
    return msg;
}

void printMessage(message *msg){
    printf("i: %d\n",msg->i);
    printf("pid: %d\n",msg->pid);
    printf("tid: %ld\n",msg->tid);
    printf("dur: %d\n",msg->dur);
    printf("pl: %d\n",msg->pl);
}

int main(int argc, char *argv[]) {
    flags flags;
    message message;
    int fd;

    initFlags(&flags);
    setFlags(argc,argv,&flags);


    char* aux = malloc(sizeof(char)*100);
    sprintf(aux,"/tmp/%s",flags.fifoname);
    
    if((fd = open(aux,O_WRONLY)) == -1){
        printf("No server fifo, can't make request\n");
        exit(1);
    }


    //EACH THREAD MAKES 1 REQUEST
    message = makeMessage(1,10,-1);
    write(fd,&message,sizeof(message));

    return 0;
}