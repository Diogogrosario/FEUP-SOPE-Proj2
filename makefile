all: Q1 U1

Q1: Q1.c
	gcc -Wall -pthread -o queue queue.c -o Q1 Q1.c  

U1: U1.c
	gcc -Wall -pthread -o U1 U1.c


clean:
	@rm -f *.o U1 Q1 queue