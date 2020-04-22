#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct flags{
    int nsecs;
    char* fifoname;
};

void printFlags(struct flags *flags){
    printf("nsecs: %d\n",flags->nsecs);
    printf("fifoname: %s\n",flags->fifoname);
}

void initFlags(struct flags *flags){
   flags->nsecs = 0;
}

void setFlags(int argc,char* argv[],struct flags *flags){
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

int main(int argc, char *argv[]) {
    struct flags flags;
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
    char* testSend = "GANDA FIFO";
    write(fd,testSend,11);

    return 0;
}