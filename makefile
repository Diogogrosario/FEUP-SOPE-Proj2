all: Q2 U2

Q2: Q2.c
	gcc -Wall -pthread -o queue queue.c -o Q2 Q2.c  

U2: U2.c
	gcc -Wall -pthread -o U2 U2.c


clean:
	@rm -f *.o U2 Q2 queue