# Implement this:
# TODO -DDOC_ROOT=/test

# Uncomment the next line if you don't want debug output
debug = -DDEBUG -ggdb

all: request.o main.o response.o
	gcc -std=c99 -Wall -pedantic $(debug) main.o request.o response.o -o simpleserver

main.o: src/ss.c src/ss.h
	gcc -c -std=c99 -Wall -pedantic $(debug) src/ss.c -o main.o

request.o: src/request.c src/request.h
	gcc -c -std=c99 -Wall -pedantic $(debug) src/request.c -o request.o

response.o: src/response.c src/response.h
	gcc -c -std=c99 -Wall -pedantic $(debug) src/response.c -o response.o

clean:
	rm simpleserver
	rm *.o
