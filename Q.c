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
    int nplaces;
    int nthreads;
    char* fifoname;
};

void printFlags(struct flags *flags){
    printf("nsecs: %d\n",flags->nsecs);
    printf("nplaces: %d\n",flags->nplaces);
    printf("nthreads: %d\n",flags->nthreads);
    printf("fifoname: %s\n",flags->fifoname);
}

void initFlags(struct flags *flags){
   flags->nsecs = 0;
   flags->nplaces = 0;
   flags->nthreads = 0;

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

int main(int argc, char *argv[]) {
    struct flags flags;

    initFlags(&flags);
    setFlags(argc,argv,&flags);

    char* aux = malloc(sizeof(char)*100);
    sprintf(aux,"/tmp/%s",flags.fifoname);
    mkfifo(aux,0666);
    
    int fd = open(aux,O_RDONLY);
    char* testSend = malloc(sizeof(char)*100);
    read(fd,testSend,11);
    printf("%s\n",testSend);

    close(fd);

    return 0;
}