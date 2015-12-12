#include "ss.h"
#include "helper_macros.h"

int main (int argc, char *argv[])
{
	int sfd;
	int cfd;

	char *request;
	int request_size;

	struct in_addr inet_addr;
	struct sockaddr_in sock_addr;

	uint32_t ip_addr_local;
	uint16_t port_local;

	int i;
	char test[] = "HTTP/1.1 200 OK\n\n<!doctype html><html><h1>Test lol</h1><img src=\"test.jpg\" /></html>";

	ip_addr_local = 3232236134; /* 192.168.2.102 */
	port_local = 12345;

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		return -1;

	inet_addr.s_addr = htonl(ip_addr_local);

	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port_local);
	sock_addr.sin_addr = inet_addr;

	test(bind(sfd, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_in)));
	
	test(listen(sfd, 1)); /* 1 is the backlog */

	for(i = 0; i<2; i++) {
		request = malloc(REQUEST_SIZE);
		test_mem(request);
		memset(request, '\0', REQUEST_SIZE);

		cfd = accept(sfd, NULL, 0);
		request_size = 0;
		while(read(cfd, request + request_size, REQUEST_SIZE) == REQUEST_SIZE) {
			request_size += REQUEST_SIZE;
			request = realloc(request, REQUEST_SIZE + request_size);
			test_mem(request);
		}

		debug_s("", request);

		//TODO process_request(request);
		//TODO make_response();
		write(cfd, test, strlen(test));

		close(cfd);
		memset(request, '\0', REQUEST_SIZE + request_size);
		free(request);
	}

	close(sfd);

	return 0;
}

