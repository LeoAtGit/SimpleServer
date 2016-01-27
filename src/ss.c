#include "ss.h"

int main (int argc, char *argv[])
{
	int sfd = 0;
	int cfd = 0;
	int fd = -1;
	FILE *lfd = NULL;

	char *request_string = NULL;
	int request_size;
	struct request_struct *request = NULL;
	struct response_struct *response = NULL;

	struct in_addr inet_addr;
	struct sockaddr_in sock_addr;

	uint32_t ip_addr_local = 2130706433; /* 127.0.0.1 => bind to localhost */

	/* deamonize the process.
	 * Taken from "The Linux Programming Interface", Listing 37-2
	 */
	switch (fork()) {
		case -1:
			return -1;
		case 0:
			break;
		default:
			_exit(EXIT_SUCCESS);
	}

	if (setsid() == -1)
		return -1;

	switch (fork()) {
		case -1:
			return -1;
		case 0:
			break;
		default:
			_exit(EXIT_SUCCESS);
	}

	close(STDIN_FILENO);
	fd = open("/dev/null", O_RDWR);
	if (fd != STDIN_FILENO)
		return -1;

	if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
		return -1;

	if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
		return -1;

	test(load_config());
	test(init_request_method_array());
	test(init_supported_versions_array());

	if (cfg.log) {
		lfd = fopen(cfg.logfile, "a");
		test_mem(lfd);
	}
	
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		goto error;

	request = malloc(sizeof(struct request_struct));
	test_mem(request);
	memset(request, '\0', sizeof(struct request_struct));

	response = malloc(sizeof(struct response_struct));
	test_mem(response);
	memset(response, '\0', sizeof(struct response_struct));

	inet_addr.s_addr = htonl(ip_addr_local);

	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(cfg.port);
	sock_addr.sin_addr = inet_addr;

	test(bind(sfd, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_in)));
	
	test(listen(sfd, SOMAXCONN));

	for (;;) {
		request_string = malloc(cfg.request_size);
		test_mem(request_string);
		memset(request_string, '\0', cfg.request_size);

		cfd = accept(sfd, NULL, 0);
		request_size = 0;

		/* We first have to do a blocking read call, so that we don't
		 * just get empty data because recv doesn't wait for the data
		 * to arrive */
		if (read(cfd, request_string, cfg.request_size) == cfg.request_size) {
			request_size += cfg.request_size;
			request_string = realloc(request_string, cfg.request_size + request_size);
			test_mem(request_string);
			memset(request_string + request_size, '\0', cfg.request_size);
			
			/* Non-blocking recv calls */
			while (recv(cfd, request_string + request_size, cfg.request_size, MSG_DONTWAIT) == cfg.request_size) {
				/* prevent too big requests */
				if (request_size > cfg.max_request_size) {
					free(request_string);
					request_string = NULL;
					break;
				}

				request_size += cfg.request_size;
				request_string = realloc(request_string, cfg.request_size + request_size);
				test_mem(request_string);
				memset(request_string + request_size, '\0', cfg.request_size);
			}
		}

		if (request_string != NULL) {
			response->status_code = process_request(request_string, request);
		} else {
			response->status_code = 500;
		}

		if (make_response(response, request, cfd, cfg.doc_root)) {
			response->status_code = 500;
			/* Try it one more time with different status code */
			make_response(response, request, cfd, cfg.doc_root);
		}

		if (cfg.log && request_string) {
			fwrite(request_string, strlen(request_string), 1, lfd);
		}

		close(cfd);
		free(response->message_body);
		free(request->method);
		free(request->request_uri);
		free(request->http_version);
		free(request->request_header);
		memset(request_string, '\0', cfg.request_size + request_size);
		free(request_string);
	}

	if (cfg.log)
		fclose(lfd);
	if (sfd)
		close(sfd);
	free(response);
	free(request);
	free(request_method_array);
	free(supported_versions_array);
	free(cfg.logfile);
	free(cfg.doc_root);
	return 0;

error:
	if (cfg.log && lfd) {
		fwrite("Error Exit!", strlen("Error Exit!"), 1, lfd);
		fclose(lfd);
		free(cfg.logfile);
		free(cfg.doc_root);
	}
	if (sfd)
		close(sfd);
	if (request) {
		free(request->method);
		free(request->request_uri);
		free(request->http_version);
		free(request->request_header);
	}
	free(request);
	free(response);
	free(request_method_array);
	free(supported_versions_array);
	free(request_string);
	return -1;
}

int load_config (void)
{
	FILE *cfd;
	char *lineptr;
	size_t n;
	int count;
	int i;
	int strlentmp;
	char *tmp;

	lineptr = NULL;
	tmp = NULL;

	cfd = fopen("config.ini", "r");
	test_mem(cfd);

	count = CONFIG_OPTIONS;
	lineptr = "";
	n = 0;

	/* Default options */
	cfg.port = 8080;
	cfg.request_size = 25;
	cfg.max_request_size = 65535;
	cfg.log = 1;
	cfg.logfile = "ss.log";
	cfg.doc_root = "html";

	while (getline(&lineptr, &n, cfd) != -1) {
		if (lineptr[0] == '#') /* Ignore the rest of the line, because it's a comment */
			continue;

		strlentmp = 0;
		/* has to be at the beginning of the line */
		if (strstr(lineptr, "port=") == lineptr) {
			for (i = strlen("port="); lineptr[i] != '\0'; i++) {
				strlentmp++;
			}
			if (strlentmp != 0) {
				tmp = malloc(sizeof(char) * strlentmp);
				test_mem(tmp);
				memset(tmp, '\0', sizeof(char) * strlentmp);
				strncpy(tmp, lineptr + strlen("port="), sizeof(char) * strlentmp);
				cfg.port = (uint16_t) atoi(tmp);
				free(tmp);
				tmp = NULL;
			}
			strlentmp = 0;
		} else if (strstr(lineptr, "request_size=") == lineptr) {
			for (i = strlen("request_size="); lineptr[i] != '\0'; i++) {
				strlentmp++;
			}
			if (strlentmp != 0) {
				tmp = malloc(sizeof(char) * strlentmp);
				test_mem(tmp);
				memset(tmp, '\0', sizeof(char) * strlentmp);
				strncpy(tmp, lineptr + strlen("request_size="), sizeof(char) * strlentmp);
				cfg.request_size = (size_t) atol(tmp);
				free(tmp);
				tmp = NULL;
			}
			strlentmp = 0;
		} else if (strstr(lineptr, "max_request_size=") == lineptr) {
			for (i = strlen("max_request_size="); lineptr[i] != '\0'; i++) {
				strlentmp++;
			}
			if (strlentmp != 0) {
				tmp = malloc(sizeof(char) * strlentmp);
				test_mem(tmp);
				memset(tmp, '\0', sizeof(char) * strlentmp);
				strncpy(tmp, lineptr + strlen("max_request_size="), sizeof(char) * strlentmp);
				cfg.max_request_size = atoi(tmp);
				free(tmp);
				tmp = NULL;
			}
			strlentmp = 0;
		} else if (strstr(lineptr, "log=") == lineptr) {
			for (i = strlen("log="); lineptr[i] != '\0'; i++) {
				strlentmp++;
			}
			if (strlentmp != 0) {
				tmp = malloc(sizeof(char) * strlentmp);
				test_mem(tmp);
				memset(tmp, '\0', sizeof(char) * strlentmp);
				strncpy(tmp, lineptr + strlen("log="), sizeof(char) * strlentmp);
				cfg.log = atoi(tmp);
				free(tmp);
				tmp = NULL;
			}
			strlentmp = 0;
		} else if (strstr(lineptr, "logfile=") == lineptr) {
			for (i = strlen("logfile="); lineptr[i] != '\0'; i++) {
				strlentmp++;
			}
			if (strlentmp != 0) {
				tmp = malloc(sizeof(char) * strlentmp);
				test_mem(tmp);
				memset(tmp, '\0', sizeof(char) * strlentmp);
				strncpy(tmp, lineptr + strlen("logfile="), sizeof(char) * (strlentmp - 1));
				cfg.logfile = tmp;
			}
			strlentmp = 0;
		} else if (strstr(lineptr, "docroot=") == lineptr) {
			for (i = strlen("docroot="); lineptr[i] != '\0'; i++) {
				strlentmp++;
			}
			if (strlentmp != 0) {
				tmp = malloc(sizeof(char) * strlentmp);
				test_mem(tmp);
				memset(tmp, '\0', sizeof(char) * strlentmp);
				strncpy(tmp, lineptr + strlen("docroot="), sizeof(char) * (strlentmp - 1));
				cfg.doc_root = tmp;
			}
			strlentmp = 0;
		}
		count--;
	}

	fclose(cfd);
	free(lineptr);

	return 0;
error:
	free(tmp);
	fclose(cfd);
	free(lineptr);
	return -1;
}

int init_supported_versions_array (void)
{
	supported_versions_array = malloc(sizeof(char *) * (SUPPORTED_VERSIONS + 1));
	test_mem(supported_versions_array);

	supported_versions_array[0] = "HTTP/1.1";
	supported_versions_array[SUPPORTED_VERSIONS] = NULL;

	return 0;

error:
	return 1;
}

int init_request_method_array (void)
{
	request_method_array = malloc(sizeof(char *) * (REQUEST_METHODS + 1));
	test_mem(request_method_array);

	request_method_array[0] = "GET";
	request_method_array[REQUEST_METHODS] = NULL;

	return 0;

error:
	return 1;
}

