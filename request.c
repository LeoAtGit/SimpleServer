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

	elements = split_words(request_string);
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
	goto error;
}

/*
 * split_words - splits the first line of words of a @text in a array of single
 * words and returns this array. The last element is NULL.
 *
 * Returns array of strings on success, NULL on failure
 *
 * Mallocs and Frees (only on failure): tmp
 *                                      result
 * Mallocs: tmp => element of result
 *          result => is returned
 */
char **split_words(char *text)
{
	int i, beginning, wordlen, num_of_words;
	char *tmp;
	char **result;

	num_of_words = 0;

	for (i = 0; text[i] != '\0'; i++) {
		if (text[i] == ' ') 
			continue;
		else
			break;
	}

	/* No useful data in the text */
	if (text[i] == '\0')
		return NULL;

	result = malloc(sizeof(char *));
	test_mem(result);

	/* Now we know the current text[i] is something we can work with */
	while (text[i] != '\0' && text[i] != '\r' && text[i] != '\n') {
		beginning = i;

		/* skip over the word */
		for (; text[i] != '\0' && text[i] != ' ' && text[i] != '\r' && text[i] != '\n'; i++) {}
		wordlen = i - beginning;
	
		tmp = malloc(sizeof(char) * (wordlen + 1));
		test_mem(tmp);
		memset(tmp, '\0', sizeof(char) * (wordlen + 1));

		if (strncpy(tmp, text + beginning, wordlen) == NULL)
			goto error;

		result[num_of_words] = tmp;
		num_of_words++;

		/* skip the space */
		for (; text[i] == ' '; i++) {}

		/* 1 is the memory of the first one */
		result = realloc(result, sizeof(char *) * (1 + num_of_words)); 
		test_mem(result);
	}

	/* Add the Request Header to result */
	if (text[i] != '\0') {
		if (text != NULL) {
			wordlen = strlen(text) - i;
		} else {
			goto error;
		}
		tmp = malloc(sizeof(char) * (wordlen + 1));
		test_mem(tmp);
		memset(tmp, '\0', sizeof(char) * (wordlen + 1));

		if (strncpy(tmp, text + i, wordlen) == NULL)
			goto error;

		result[num_of_words] = tmp;
		num_of_words++;

		result = realloc(result, sizeof(char *) * (1 + num_of_words));
		test_mem(result);
	}

	result[num_of_words] = NULL;
	return result;

error:
	free(tmp);
	for (i = 0; i <= num_of_words; i++) {
		free(result[i]);
	}
	free(result);

	return NULL;
}

