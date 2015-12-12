# Uncomment the next line if you don't want debug output
debug = -DDEBUG -ggdb

all:
	gcc -std=c99 -Wall $(debug) ss.c -o ss

clean:
	rm ss
