CFLAGS = -c
CC = gcc

all: homework1

homework1.o: homework1.c
	${CC} ${CFLAGS} homework1.c
homework1: homework1.o
	${CC} -o homework1 homework1.o
clean: 
	rm -f *.o *~
