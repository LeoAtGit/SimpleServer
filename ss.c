#include "ss.h"
#include "helper_macros.h"

int main (int argc, char *argv[])
{
	int sfd;
	int cfd;

	char *request_string;
	int request_size;
	struct request_struct *request;

	struct in_addr inet_addr;
	struct sockaddr_in sock_addr;

	uint32_t ip_addr_local;
	uint16_t port_local;

	int i;
	char test[] = "HTTP/1.1 200 OK\n\n<!doctype html><html><h1>Test lol</h1><img src=\"test.jpg\" /></html>";

	request_string = NULL;

	ip_addr_local = 3232236134; /* 192.168.2.102 */
	port_local = 12345;

	request = malloc(sizeof(struct request_struct));
	test_mem(request);
	memset(request, '\0', sizeof(struct request_struct));

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		return -1;

	inet_addr.s_addr = htonl(ip_addr_local);

	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port_local);
	sock_addr.sin_addr = inet_addr;

	test(bind(sfd, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_in)));
	
	test(listen(sfd, 1)); /* 1 is the backlog */ //FIXME ???

	for(i = 0; i<2; i++) { //FIXME for infinite loop
		request_string = malloc(REQUEST_SIZE);
		test_mem(request_string);
		memset(request_string, '\0', REQUEST_SIZE);

		cfd = accept(sfd, NULL, 0);
		request_size = 0;
		while(read(cfd, request_string + request_size, REQUEST_SIZE) == REQUEST_SIZE) {
			request_size += REQUEST_SIZE;
			request_string = realloc(request_string, REQUEST_SIZE + request_size);
			test_mem(request_string);
			memset(request_string + request_size, '\0', REQUEST_SIZE);
		}

		test(process_request(request_string, request));
		//TODO make_response();
		write(cfd, test, strlen(test));

		debug_s("Method", request->method);

		close(cfd);

		free(request->method);
		free(request->request_uri);
		free(request->http_version);
		free(request->request_header);

		memset(request_string, '\0', REQUEST_SIZE + request_size);
		free(request_string);
	}

	close(sfd);
	free(request);

	return 0;

error:
	close(sfd);
	free(request->method);
	free(request->request_uri);
	free(request->http_version);
	free(request->request_header);
	free(request);
	free(request_string);
	return -1;
}

/*
 * process_request - fills the @request with the data from @request_string
 *
 * Returns 0 on Success, 1 on Failure
 */
int process_request(char *request_string, struct request_struct *request)
{
	char **elements;

	elements = split_words(request_string);
	if (elements == NULL)
		goto error;

	if (elements[0] != NULL) 
		request->method = elements[0];
	else
		goto error;

	if (elements[1] != NULL) 
		request->request_uri = elements[1];
	else
		goto error;

	if (elements[2] != NULL) 
		request->http_version = elements[2];
	else
		goto error;

	/* Not required to be set by HTTP specifications, that's why there
	 * is no else condition
	 */
	if (elements[3] != NULL) 
		request->request_header = elements[3];

	free(elements);

	return 0;

error:
	free(elements);
	return 1;
}

/*
 * split_words - splits the first line of words of a @text in a array of single
 * words and returns this array. The last element is NULL.
 *
 * Remember to free the memory of the array
 *
 * Returns NULL on failure and an array of strings called @result otherwise
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
		wordlen = strlen(text) - i;
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

