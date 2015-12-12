#include <netinet/in.h> /* internet stuff, e.g. bind */
#include <sys/socket.h> /* socket() */
#include <sys/types.h> /* many stuff */
#include <signal.h> /* signal() */
#include <netdb.h> /* getaddrinfo() */
#include <arpa/inet.h> /* for htons() etc */
#include <sys/stat.h> /* open() */

#include <stdio.h>  /* printf() */
#include <string.h> /* memset() */
#include <stdlib.h> /* malloc() */
#include <unistd.h> /* sleep() */
#include <errno.h>  /* errno() */
#include <fcntl.h>  /* open() */

#define REQUEST_SIZE sizeof(char) * 25

