all: Q U

Q: Q.c
	gcc -Wall -pthread -o Q Q.c

U: U.c
	gcc -Wall -pthread -o U U.c