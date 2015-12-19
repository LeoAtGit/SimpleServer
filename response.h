#ifndef RESPONSE_H
#define RESPONSE_H

#include "request.h"

#ifndef DOC_ROOT
	#define DOC_ROOT "html"
#endif /* DOC_ROOT */

int total_bytes_read;

#define READ_SIZE sizeof(char) * 100

#define CONTINUE 100
#define OK  200
#define BAD_REQUEST  400
#define NOT_FOUND 404
#define INTERNAL_SERVER_ERROR 500
#define NOT_IMPLEMENTED 501
#define HTTP_VERSION_NOT_SUPPORTED 505

/* TODO Implement all of these
	"100" Continue
	"101" Switching Protocols
	"200" OK
	"201" Created
	"202" Accepted
	"203" Non-Authoritative Information
	"204" No Content
	"205" Reset Content
	"206" Partial Content
	"300" Multiple Choices
	"301" Moved Permanently
	"302" Found
	"303" See Other
	"304" Not Modified
	"305" Use Proxy
	"307" Temporary Redirect
	"400" Bad Request
	"401" Unauthorized
	"402" Payment Required
	"403" Forbidden
	"404" Not Found
	"405" Method Not Allowed
	"406" Not Acceptable
	"407" Proxy Authentication Required
	"408" Request Time-out
	"409" Conflict
	"410" Gone
	"411" Length Required
	"412" Precondition Failed
	"413" Request Entity Too Large
	"414" Request-URI Too Large
	"415" Unsupported Media Type
	"416" Requested range not satisfiable
	"417" Expectation Failed
	"500" Internal Server Error
	"501" Not Implemented
	"502" Bad Gateway
	"503" Service Unavailable
	"504" Gateway Time-out
	"505" HTTP Version not supported
*/

struct request_struct;
struct response_struct {
	char *http_version;
	int status_code;
	char *reason_phrase;
	void *message_body;
};

int make_response(struct response_struct *response, struct request_struct *request, int status_code, int fd);
int choose_reason_phrase(struct response_struct *response);
int generate_message_body(struct response_struct *response, char *request_uri);
int write_response(int fd, struct response_struct *response);

#endif /* RESPONSE_H */

