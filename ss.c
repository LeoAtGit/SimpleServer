#include "ss.h"
#include "helper_macros.h"
#include "request.h"
#include "response.h"

int main (int argc, char *argv[])
{
	int sfd;
	int cfd;

	char *request_string;
	int request_size;
	struct request_struct *request;

	int response_code;
	struct response_struct *response;

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

	response = malloc(sizeof(struct response_struct));
	test_mem(response);
	memset(response, '\0', sizeof(struct response_struct));

	test(init_request_method_array());
	test(init_supported_versions_array());

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

		response_code = process_request(request_string, request);
		if (make_response(response, request, response_code)) {
			/* it failed, so return a internal server error */
			//TODO
		}
		write(cfd, test, strlen(test));

		debug_s("Method", request->method);

		close(cfd);

		//TODO free response

		free(request->method);
		free(request->request_uri);
		free(request->http_version);
		free(request->request_header);

		memset(request_string, '\0', REQUEST_SIZE + request_size);
		free(request_string);
	}

	close(sfd);
	free(request);
	free(request_method_array);

	return 0;

error:
	close(sfd);
	free(request->method);
	free(request->request_uri);
	free(request->http_version);
	free(request->request_header);
	free(request);
	free(request_method_array);
	free(request_string);
	return -1;
}

int init_supported_versions_array()
{
	supported_versions_array = malloc(sizeof(char *) * (SUPPORTED_VERSIONS + 1));
	test_mem(supported_versions_array);

	supported_versions_array[0] = "HTTP/1.1";
	supported_versions_array[SUPPORTED_VERSIONS] = "HTTP/1.1";

	return 0;

error:
	return 1;
}

int init_request_method_array()
{
	request_method_array = malloc(sizeof(char *) * (REQUEST_METHODS + 1));
	test_mem(request_method_array);

	request_method_array[0] = "GET";
	request_method_array[1] = "POST";
	request_method_array[2] = "OPTIONS";
	request_method_array[3] = "HEAD";
	request_method_array[4] = "PUT";
	request_method_array[5] = "DELETE";
	request_method_array[6] = "TRACE";
	request_method_array[7] = "CONNECT";
	request_method_array[REQUEST_METHODS] = NULL;

	return 0;

error:
	return 1;
}

