#ifndef REQUEST_H
#define REQUEST_H

#include "ss.h"
#include "helper_macros.h"

#define REQUEST_METHODS 8
char **request_method_array;

int process_request(char *request_string, struct request_struct *request);
char **split_words(char *text);
int in_array(const char *element, char **array);

#endif /* REQUEST_H */

