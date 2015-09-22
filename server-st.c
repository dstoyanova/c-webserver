/**
 * server-st.c: Simple, single-threaded web server implementation.
 *
 * Repeatedly handles HTTP requests sent to this port number.
 * Most of the work is done within routines written in request.c
 *
 * Course: 1DT032
 * 
 * To run:
 *  server <portnum (above 2000)>
 */

#include <assert.h>
#include "request.h"
#include "util.h"

/* Structure of a HTTP request. */
typedef struct {
	int fd;
	long size, arrival, dispatch;
} request;

/**
 * Very simple input option parser.
 *
 * @argc: number of input arguments
 * @argv: input arguments
 * @port: port number that the server will listen.
 */
void getargs(int argc, char *argv[], int *port)
{
	/* TODO: Verify that the arguments are valid */

}

int requestcmp(const void *first, const void *second) {
	assert(first != NULL);
	assert(second != NULL);
	return ((*(request **)first)->size - (*(request **)second)->size);
}

/**
 * Calculates the time in microsecond resultion.
 * 
 * ARGUMENTS:
 * @t: timeval structure
 */
long calculate_time(struct timeval t) {
	return ((t.tv_sec) * 1000 + t.tv_usec/1000.0) + 0.5;
}


/**
 * Main server loop
 */
int main(int argc, char *argv[]) {
	int listenfd, connfd, port, clientlen;
	struct sockaddr_in clientaddr;
	struct timeval arrival;

	/* Parse the input arguments */
	getargs(argc, argv, &port);

	printf("\n1DT032 server: Hello! I am running on port %d.\n\n", port);

	/* Listen to user selected port */
	listenfd = Open_listenfd(port);
	
	/* Main server loop */
	while (1) {
		clientlen = sizeof(clientaddr);

		/* TODO: Accept a connection and retrieve connfd */

		/* TODO: Allocate a request structure */

		/* TODO: Save the time for the statistics */

		/* TODO: Set the request file descriptor to the one accepted */

		/* TODO: Set the arrival and dispatch time */
		
		/* TODO: Call the request handler */

		/* TODO: Close */
	
	}

}


    


 
