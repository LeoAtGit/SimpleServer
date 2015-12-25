#include "request.h"

/*
 * in_array - checks if @element is in @array
 *
 * Returns 1 if there was a match, and 0 if there was not
 */
int in_array(const char *element, char **array)
{
	int i;

	for (i = 0; array[i]; i++) {
		if (strcmp(element, array[i]) == 0) {
			return 1;
		}
	}

	return 0;
}

/*
 * process_request - fills the @request with the data from @request_string
 *
 * Returns HTTP Status Code
 *
 * Frees: elements
 * Frees (only on failure): elements[0]
 *                          elements[1]
 *                          elements[2]
 *                          elements[3]
 */
int process_request(char *request_string, struct request_struct *request)
{
	char **elements;
	int code;

	code = OK;

	elements = convert_to_request_struct(request_string);
	if (elements == NULL) {
		code = NOT_IMPLEMENTED;
		goto error;
	}

	if (elements[0] != NULL) {
		if (in_array(elements[0], request_method_array)) {
			request->method = elements[0];
		} else {
			code = BAD_REQUEST;
			goto free_all;
		}
	} else {
		code = NOT_IMPLEMENTED;
		goto free_all;
	}

	if (elements[1] != NULL) {
		request->request_uri = elements[1];
	} else {
		code = NOT_IMPLEMENTED;
		goto free_all;
	}

	if (elements[2] != NULL) {
		if (in_array(elements[2], supported_versions_array)) {
			request->http_version = elements[2];
		} else {
			code = HTTP_VERSION_NOT_SUPPORTED;
			goto free_all;
		}
	} else {
		code = NOT_IMPLEMENTED;
		goto free_all;
	}

	/* Not required to be set by HTTP specifications, that's why there
	 * is no else condition
	 */
	if (elements[3] != NULL) {
		request->request_header = elements[3];
	}

error:
	free(elements);
	return code;

free_all:
	free(elements[0]);
	free(elements[1]);
	free(elements[2]);
	free(elements[3]);
	request->method = NULL;
	request->request_uri = NULL;
	request->http_version = NULL;
	request->request_header = NULL;
	goto error;
}

char **convert_to_request_struct(char *text)
{
	int i, tmp_int;
	char *tmp;
	char **result;

	if (!(text[0] >= 'A' && text[0] <= 'Z')) {
		return NULL;
	}

	result = calloc(5, sizeof(char *));
	test_mem(result);

	for (i = 0; text[i] >= 'A' && text[i] <= 'Z'; i++) {}

	/* Method */
	tmp = malloc(sizeof(char) * (i + 1));
	test_mem(tmp);
	memset(tmp, '\0', sizeof(char) * (i + 1));
	strncpy(tmp, text, i);
	result[0] = tmp;

	/* Request-Uri */
	if (text[i] != ' ') {
		goto free;
	}
	for (; text[i] == ' '; i++) {}
	tmp_int = i;
	for (; text[i] > ' ' && text[i] <= '~'; i++) {}
	tmp = malloc(sizeof(char) * ((i - tmp_int) + 1));
	test_mem(tmp);
	memset(tmp, '\0', sizeof(char) * ((i - tmp_int) + 1));
	strncpy(tmp, text + tmp_int, i - tmp_int);
	result[1] = tmp;

	/* HTTP Version */
	if (text[i] != ' ') {
		goto free;
	}
	for (; text[i] == ' '; i++) {}
	tmp_int = i;
	for (; text[i] >= '.' && text[i] < 'Z'; i++) {}
	tmp = malloc(sizeof(char) * ((i - tmp_int) + 1));
	test_mem(tmp);
	memset(tmp, '\0', sizeof(char) * ((i - tmp_int) + 1));
	strncpy(tmp, text + tmp_int, i - tmp_int);
	result[2] = tmp;

	/* Request Header */
	if (text[i] == '\0') {
		goto skip;
	}
	for (; text[i] == '\n' || text[i] == '\r'; i++) {}
	tmp_int = i;
	for (; text[i] != '\0'; i++) {}
	tmp = malloc(sizeof(char) * ((i - tmp_int) + 1));
	test_mem(tmp);
	memset(tmp, '\0', sizeof(char) * ((i - tmp_int) + 1));
	strncpy(tmp, text + tmp_int, i - tmp_int);
	result[3] = tmp;

skip:
	result[4] = NULL;
	return result;

error:
free:
	free(result[0]);
	free(result[1]);
	free(result[2]);
	free(result[3]);
	free(result);
	return NULL;

}

