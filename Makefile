
CFLAGS = -std=c89 -O3
CC = gcc

.PHONY: default clean

default: lambda

lambda: lambda.o start.o
	$(CC) -o $@ lambda.o start.o

liblambda: lambda.o start.o
	ar rcs $@ lambda.o start.o

clean:
	rm -f *.o lambda liblambda.a

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<
