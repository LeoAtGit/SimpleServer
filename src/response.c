#include "response.h"

/* 
 * make_response - main function for filling the @response with the correct 
 * information gotten from @request and sending it back to the file descriptor
 * @fd of the client. If the status code is not OK (200), then don't even do
 * rest, since there was a previous error.
 *
 * Returns 0 on success and -1 on failure
 */
int make_response(struct response_struct *response, struct request_struct *request, int fd, char *doc_root)
{
	response->message_body = "";
	response->http_version = "HTTP/1.1";

	if (response->status_code == OK) {
		choose_reason_phrase(response);
		if (generate_message_body(response, request->request_uri, doc_root) == -1) {
			goto error_template;
		}
	} else {
error_template:
		choose_reason_phrase(response);
		generate_error_template(response);
	}

	if (write_response(fd, response) != 0) {
		debug_s("write_response", "No successful write\n");
		return -1;
	}

	return 0;
}

/*
 * generate_error_template - if there was a error status code this function
 * generates the message_body of the response
 *
 * Always returns 0
 *
 * Mallocs: response_body => passed to response->message_body
 */
int generate_error_template(struct response_struct *response)
{
	char *response_body;

	char HTML_START[] = "<!doctype html><html><body>";
	char HTML_END[] = "</body></html>";
	int STRLEN_HTML_WRAPPER = strlens(HTML_START) + strlens(HTML_END);

	choose_reason_phrase(response);
	response_body = malloc(sizeof(char) * (STRLEN_HTML_WRAPPER + strlens(response->reason_phrase) + 1));
	test_mem(response_body);
	memset(response_body, '\0', sizeof(char) * (STRLEN_HTML_WRAPPER + strlens(response->reason_phrase) + 1));

	strncpy(response_body, HTML_START, strlens(HTML_START));
	strncpy(response_body + strlens(HTML_START), response->reason_phrase, strlens(response->reason_phrase));
	strncpy(response_body + strlens(HTML_START) + strlens(response->reason_phrase), HTML_END, strlens(HTML_END));

	response->message_body = response_body;
	total_bytes_read = strlens(response->message_body);

error:
	return 0;
}

/*
 * generate_message_body - looks what the request uri from @request_uri is,
 * performs some checks to see whether you are allowed to see that file and
 * saves it finally in the @response->message_body
 *
 * Returns 0 on success and -1 on failure
 *
 * Mallocs and Frees: filepath
 * Mallocs: response_body => passed to response->message_body
 */
int generate_message_body(struct response_struct *response, char *request_uri, char *doc_root)
{
	char *filepath;
	int docroot_strlen;
	int request_strlen;

	int fd_request;
	char *response_body;
	int response_size;
	int read_bytes;

	struct stat stat_buf;

	total_bytes_read = 0;
	read_bytes = 0;
	fd_request = -1;
	filepath = NULL;
	response_body = NULL;
	memset(&stat_buf, '\0', sizeof(struct stat));

	if (strcmp(request_uri, "/") == 0) {
		request_uri = "/index.html";
	}

	docroot_strlen = strlens(doc_root);
	request_strlen = strlens(request_uri);

	filepath = malloc(sizeof(char) * (docroot_strlen + request_strlen + 1));
	test_mem(filepath);
	strncpy(filepath, doc_root, docroot_strlen);
	strncpy(filepath + docroot_strlen, request_uri, request_strlen);
	filepath[docroot_strlen + request_strlen] = '\0';

	filepath = clean_filepath(filepath);
	if (filepath == NULL) {
		response->status_code = INTERNAL_SERVER_ERROR;
		goto error;
	}

	stat(filepath, &stat_buf);
	if (S_ISDIR(stat_buf.st_mode)) {
		response->status_code = NOT_FOUND;
		goto error;
	} else if (S_ISREG(stat_buf.st_mode)) {
		fd_request = open(filepath, O_RDONLY);
	}

	if (fd_request == -1) {
		/* File does not exist */
		response->status_code = NOT_FOUND;
		goto error;
	}

	response_body = malloc(READ_SIZE);
	test_mem(response_body);
	memset(response_body, '\0', READ_SIZE);

	response_size = 0;

	do {
		read_bytes = read(fd_request, response_body + response_size, READ_SIZE);
		response_size += read_bytes;
		response_body = realloc(response_body, READ_SIZE + response_size);
		test_mem(response_body);
		memset(response_body + response_size, '\0', READ_SIZE);
	} while (read_bytes == READ_SIZE);

	total_bytes_read = response_size;
	response->message_body = response_body;

	close(fd_request);
	free(filepath);
	return 0;

error:
	if (fd_request != -1)
		close(fd_request);
	free(filepath);
	return -1;
}

/*
 * write_response - puts all the pieces from @response together and sends it to
 * the client file descriptor @fd
 *
 * Returns 0 on success and -1 on failure
 *
 * Mallocs and Frees: response_str
 */
int write_response(int fd, struct response_struct *response)
{
	int total_strlen;
	int bytes_written;
	char *response_str;
	char response_status_code[4];

	snprintf(response_status_code, 4, "%d", response->status_code);

	total_strlen = strlens(response->http_version) + strlens(response_status_code) + strlens(response->reason_phrase) + total_bytes_read + 5; /* 5 = " " and " " and "\n\n" and "\0" */

	response_str = malloc(sizeof(char) * total_strlen);
	test_mem(response_str);
	memset(response_str, '\0', sizeof(char) * total_strlen);

	bytes_written = 0;

	strncpy(response_str + bytes_written, response->http_version, strlens(response->http_version));
	bytes_written += strlens(response->http_version);
	response_str[bytes_written] = ' ';
	bytes_written++;
	
	strncpy(response_str + bytes_written, response_status_code, strlens(response_status_code));
	bytes_written += strlens(response_status_code);
	response_str[bytes_written] = ' ';
	bytes_written++;
	
	strncpy(response_str + bytes_written, response->reason_phrase, strlens(response->reason_phrase));
	bytes_written += strlens(response->reason_phrase);
	response_str[bytes_written] = '\n';
	bytes_written++;
	response_str[bytes_written] = '\n';
	bytes_written++;

	bcopy(response->message_body, response_str + bytes_written, total_bytes_read);
	bytes_written += total_bytes_read;
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

char *clean_filepath (char *filepath)
{
	char *tmp;
	int total_len, tmp_len, len1, len2;

	for (;;) {
		total_len = strlen(filepath);
		if (total_len < 3)
			break;
		tmp = strstr(filepath, "../");
		if (tmp == NULL) /* No more occurence of "../" */
			break;
		tmp_len = strlen(tmp);
		len1 = total_len - tmp_len;
		len2 = len1 + 3;

		tmp = malloc(sizeof(char) * (total_len - 2));
		test_mem(tmp);
		memset(tmp, '\0', sizeof(char) * (total_len - 2));

		snprintf(tmp, total_len - 2, "%.*s%s", len1, filepath, filepath + len2);

		free(filepath);
		filepath = tmp;
	}

	return filepath;
error:
	free(filepath);
	return NULL;
}

/*
 * choose_reason_phrase - Checks the status code from @response->status_code and
 * sets the reason phrase for the response accordingly
 *
 * Returns 0 on success
 */
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
		case FORBIDDEN:
			response->reason_phrase = "Forbidden";
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
			response->status_code = NOT_IMPLEMENTED;
			response->reason_phrase = "Not Implemented";
	}
	return 0;
}

