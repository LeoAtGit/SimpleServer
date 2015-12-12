#include "ss.h"
#include "helper_macros.h"

int main (int argc, char *argv[])
{
	int sfd;
	int cfd;
	char buffer[1000];

	struct in_addr inet_addr;
	struct sockaddr_in sock_addr;

	uint32_t ip_addr_local;
	uint16_t port_local;

	char test[] = "HTTP/1.1 200 OK\n\n<!doctype html><html><h1>Test lol</h1></html>";

	ip_addr_local = 3232236134; /* 192.168.2.102 */
	port_local = 12345;

	memset(buffer, '\0', 1000);

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		return -1;

	inet_addr.s_addr = htonl(ip_addr_local);

	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port_local);
	sock_addr.sin_addr = inet_addr;

	test(bind(sfd, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_in)));
	
	test(listen(sfd, 1)); /* 1 is the backlog */

	cfd = accept(sfd, NULL, 0);
	//for(;;) {
		read(cfd, buffer, 1000);
		write(cfd, test, strlen(test));

		//printf("%s", buffer);
		debug_s("", buffer);

		//if (bytes_read == 0)
			//break;
	//}

	close(cfd);
	close(sfd);

	return 0;
}

