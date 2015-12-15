# Uncomment the next line if you don't want debug output
debug = -DDEBUG -ggdb

all: request.o main.o
	gcc -std=c99 -Wall -pedantic $(debug) request.o main.o -o simpleserver

main.o: ss.c ss.h
	gcc -c -std=c99 -Wall -pedantic $(debug) ss.c -o main.o

request.o: request.c request.h
	gcc -c -std=c99 -Wall -pedantic $(debug) request.c -o request.o

clean:
	rm simpleserver
	rm *.o
