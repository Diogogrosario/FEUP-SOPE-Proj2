all: Q1 U1

Q2: Q1.c
	gcc -Wall -pthread -o queue queue.c -o Q2 Q2.c  

U2: U1.c
	gcc -Wall -pthread -o U2 U2.c


clean:
	@rm -f *.o U2 Q2 queue