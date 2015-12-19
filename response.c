#include "response.h"

int make_response(struct response_struct *response, struct request_struct *request, int status_code, int fd)
{
	response->message_body = "";
	response->http_version = request->http_version; //FIXME don't lie here
	if (status_code == OK)
		response->status_code = status_code;
	else
		goto exit;
	
	generate_message_body(response, request->request_uri);

exit:
	if (choose_reason_phrase(response) != 0) {
		debug_s("choose_reason_phrase", "No valid case found!\n");
		return -1;
	}

	if (write_response(fd, response) != 0) {
		debug_s("write_response", "No successful write\n");
		return -1;
	}

	return 0;
}

int generate_message_body(struct response_struct *response, char *request_uri)
{
	char *filepath;
	int docroot_strlen;
	int request_strlen;

	docroot_strlen = strlen(DOC_ROOT);
	request_strlen = strlen(request_uri);

	filepath = malloc(sizeof(char) * (docroot_strlen + request_strlen + 1));
	test_mem(filepath);
	strncpy(filepath, DOC_ROOT, docroot_strlen);
	strncpy(filepath + docroot_strlen, request_uri, request_strlen);
	filepath[request_strlen + 1] = '\0';

	//TODO check privileges

	response->message_body = "<!doctype html><html><h1>Test haha</h1><img src=\"test.jpg\" /></html>";

	return 0;

error:
	return -1;
}

int write_response(int fd, struct response_struct *response)
{
	int total_strlen;
	int bytes_written;
	char *response_str;
	char response_status_code[4];

	snprintf(response_status_code, 4, "%d", response->status_code);

	total_strlen = strlen(response->http_version) + strlen(response_status_code) + strlen(response->reason_phrase) + strlen(response->message_body) + 5; /* 5 = " " and "\n\n" and "\0" */

	if (total_strlen)
		response_str = malloc(sizeof(char) * total_strlen);
	test_mem(response_str);
	memset(response_str, '\0', sizeof(char) * total_strlen);

	bytes_written = 0;

	strncpy(response_str + bytes_written, response->http_version, strlen(response->http_version));
	bytes_written += strlen(response->http_version);
	response_str[bytes_written] = ' ';
	bytes_written++;
	
	strncpy(response_str + bytes_written, response_status_code, strlen(response_status_code));
	bytes_written += strlen(response_status_code);
	response_str[bytes_written] = ' ';
	bytes_written++;
	
	strncpy(response_str + bytes_written, response->reason_phrase, strlen(response->reason_phrase));
	bytes_written += strlen(response->reason_phrase);
	response_str[bytes_written] = '\n';
	bytes_written++;
	response_str[bytes_written] = '\n';
	bytes_written++;

	strncpy(response_str + bytes_written, response->message_body, strlen(response->message_body));
	bytes_written += strlen(response->message_body);
	response_str[bytes_written] = '\0';

	if (write(fd, response_str, total_strlen) != total_strlen) {
		debug_s("write", "failed, didn't write all of the data\n");
		goto error;
	}

	free(response_str);
	return 0;

error:
	free(response_str);
	return -1;
}

int choose_reason_phrase(struct response_struct *response)
{
	switch (response->status_code) {
		case CONTINUE:
			response->reason_phrase = "Continue";
			break;
		case OK:
			response->reason_phrase = "OK";
			break;
		case BAD_REQUEST:
			response->reason_phrase = "Bad Request";
			break;
		case NOT_FOUND:
			response->reason_phrase = "Not Found";
			break;
		case INTERNAL_SERVER_ERROR:
			response->reason_phrase = "Internal Server Error";
			break;
		case NOT_IMPLEMENTED:
			response->reason_phrase = "Not Implemented";
			break;
		case HTTP_VERSION_NOT_SUPPORTED:
			response->reason_phrase = "HTTP Version not supported";
			break;
		default:
			return -1;
	}
	return 0;
}
