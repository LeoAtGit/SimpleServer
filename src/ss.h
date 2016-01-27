#ifndef SS_H
#define SS_H

#define _XOPEN_SOURCE 700

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

#include "request.h"
#include "helper_macros.h"
#include "response.h"

#define CONFIG_OPTIONS 5

struct config {
	uint16_t port;
	size_t request_size;  /* For each individual read of input stream */
	int max_request_size; /* In total */
	int log;
	char *logfile;
	char *doc_root;
} cfg;

#define SUPPORTED_VERSIONS 1
char **supported_versions_array;

int load_config(void);
int init_request_method_array(void);
int init_supported_versions_array(void);

#endif /* SS_H */

