# Uncomment the next line if you don't want debug output
debug_support = -DDEBUG
debugger = -ggdb

all:
	gcc -std=c99 -Wall $(debugger) $(debug_support) ss.c -o ss

clean:
	rm ss
