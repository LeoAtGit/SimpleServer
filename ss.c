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
		}

		//debug_s("", request_string);

		test(process_request(request_string, request));
		//TODO make_response();
		write(cfd, test, strlen(test));

		close(cfd);
		//TODO free all of the stuff from request
		memset(request, '\0', sizeof(struct request_struct));
		memset(request_string, '\0', REQUEST_SIZE + request_size);
		free(request_string);
	}

	free(request);
	close(sfd);

	return 0;
}

int process_request(char *request_string, struct request_struct *request)
{
	char **words;
	int i;

	words = split_words(request_string);

	for(i = 0; words[i] != NULL; i++) {
		debug_s("words", words[i]);
	}

	return 0;
}

/*
 * split_words - splits the first line of words of a @text in a array of single
 * words and returns this array. The last element is NULL.
 *
 * Be careful that you free the memory of the array
 *
 * TODO When returning NULL, free the memory before returning
 * TODO Don't just neglect the rest of the message after a '\r', '\n' or '\0'
 * 	has been found
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
			return NULL;

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
			return NULL;

		result[num_of_words] = tmp;
		num_of_words++;

		result = realloc(result, sizeof(char *) * (1 + num_of_words));
		test_mem(result);
	}

	result[num_of_words] = NULL;
	return result;
}

