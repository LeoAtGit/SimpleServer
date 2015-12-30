#ifndef GLOBALS_H
#define GLOBALS_H

/* Values that can be changed by the config file */
uint16_t PORT = 8080;
size_t REQUEST_SIZE = sizeof(char) * 25;
int LOG = 1;
char *LOGFILE = "ss.log";
char *DOC_ROOT = "html";

#define SUPPORTED_VERSIONS 1
#define REQUEST_METHODS 1

/* global for getting the size of the data of message_body */
int total_bytes_read = 0;

#endif /* GLOBALS_H */

