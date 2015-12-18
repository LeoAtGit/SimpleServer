#include "response.h"

int make_response(struct response_struct *response, struct request_struct *request, int status_code, int fd)
{
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

int choose_reason_phrase(struct response_struct *response)
{
	switch (response->status_code) {
		case OK:
			response->reason_phrase = "OK";
			break;
		//FIXME etc...

		default:
			return -1;
	}
	return 0;
}

int generate_message_body(struct response_struct *response, char *request_uri)
{
	//FIXME this is just proof of concept
	response->message_body = "<!doctype html><html><h1>Test haha</h1><img src=\"test.jpg\" /></html>";
	return 0;
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

