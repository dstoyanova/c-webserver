/*
 * client.c: A very, very primitive HTTP client.
 * 
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 * For testing your server, you might want to modify this client.
 *
 * Course: 1DT032   
 *
 */

#include "util.h"

/*
 * Send an HTTP request for the specified file 
 */
void clientSend(int fd, char *filename)
{
	char buf[MAXLINE];
	char hostname[MAXLINE];

	Gethostname(hostname, MAXLINE);

	/* Form and send the HTTP request */
	sprintf(buf, "GET %s HTTP/1.1\n", filename);
	sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
	Rio_writen(fd, buf, strlen(buf));
}
  
/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd, int print_content)
{
	rio_t rio;
	char buf[MAXBUF];  
	int length = 0;
	int n;
  
	Rio_readinitb(&rio, fd);

	/* Read and display the HTTP Header */
	n = Rio_readlineb(&rio, buf, MAXBUF);
	while (strcmp(buf, "\r\n") && (n > 0)) {
		printf("Header: %s", buf);
		n = Rio_readlineb(&rio, buf, MAXBUF);

		/* If you want to look for certain HTTP tags... */
		if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
			printf("Length = %d\n", length);
		}
	}
	printf("\nContent:\n");


	/* Read and display the HTTP Body */
	n = Rio_readlineb(&rio, buf, MAXBUF);
	while (n > 0) {
		if (print_content) {
			printf("%s", buf);
		}
		n = Rio_readlineb(&rio, buf, MAXBUF);
	}
}

int main(int argc, char *argv[])
{
	char *host = NULL, *filename = NULL;
	int port = 0;
	int clientfd = 0;
	int print_content = 0;

	if (argc != 5) {
		fprintf(stderr, "Usage: %s <host> <port> <filename> <print_content [yes/no]>\n", argv[0]);
		exit(1);
	} 

	if (strcasecmp(argv[4], "yes") == 0) {
		print_content = 1;
	} else if (strcasecmp(argv[4], "no") == 0) {
		print_content = 0;
	} else {
		fprintf(stderr, "Print content option must be 'yes' or 'no'\n");
		exit(1);
	}
	
	host = argv[1];
	port = atoi(argv[2]);
	filename = argv[3];

	
	/* Open a single connection to the specified host and port */
	clientfd = Open_clientfd(host, port);
  
	clientSend(clientfd, filename);
	clientPrint(clientfd, print_content);
    
	Close(clientfd);

	exit(0);
}
