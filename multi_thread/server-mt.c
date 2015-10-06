/**
 * server-mt.c: Multi-threaded web server implementation.
 *
 * Repeatedly handles HTTP requests sent to this port number.
 * Most of the work is done within routines written in request.c
 *
 * Course: 1DT032
 * 
 * To run:
 *  server <portnum (above 2000)> <threads> <schedalg>
 */

#include <assert.h>
#include "server_types.h"
#include "request.h"
#include "util.h"

/* Mutex for the request buffer */
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* Condition variable to use when the queue was filled */
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;

/* Condition variable to use when the queue was emptied */
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

pthread_t **cid;

/* Structure of a HTTP request. */
typedef struct {
	int fd;
	long size;
	long arrival, dispatch;
} request;


/* Request queue */
volatile request **buffer;

/* Index to the next empty slot in the queue */
volatile int fillptr;

/* Index to the next request to be handled in the queue */
volatile int useptr;

/* Global variable for the size of the queue */
int max;

/* Number of clients queued */
volatile int numfull;

/* Global variable for the scheduling algorithm */
sched_alg algorithm;

/* Global variable for the id of the dispatching thread */
volatile int threadid = 0; 

/* Statistics for the average */
volatile int clients_treated = 0;
volatile int latencies_acc = 0;

/**
 * Very simple input option parser.
 *
 * @argc: 
 * @argv:
 * @port: port number that the server will listen.
 */
void getargs(int argc, char *argv[], int *port, int *threads, int *buffers, sched_alg *alg)
{
	assert(port != NULL);
	assert(threads != NULL);
	assert(buffers != NULL);
	assert(alg != NULL);
	assert(argc >=0);
	assert(argv != NULL);

	if (argc != 4) {
		fprintf(stderr, "Usage: %s <port> <threads> <schedalg>\n", argv[0]);
		exit(1);
	}
	
	*port = atoi(argv[1]);
	*threads = atoi(argv[2]);
	/* We set the number of buffers==threads */
	*buffers = atoi(argv[2]);

	if(strcasecmp(argv[3], "FIFO") == 0) {
		*alg = FIFO;
	} else if(strcasecmp(argv[3], "SFF") == 0) {
		*alg = SFF;
	} else {
		fprintf(stderr, "Scheduling algorithm must be one of the following options: FIFO, STACK, SFF, BFF.\n");
		exit(1);
	}
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
 * Consumer. The function that will be executed by each of the threads
 * created in main. (Entry point)
 * 
 */
void *consumer(void *arg) {
	assert(arg != NULL);

  	/* TODO: Create a thread structure */
  	thread worker;
  	
	/* TODO: Initialize the statistics of the thread structure */
	worker.id = 1;
	worker.count = 0;
	worker.statics = 0;
	worker.dynamics = 0;
	worker.client_id = 0;

	request *req = NULL;
	struct timeval dispatch;

	/* Main thread loop */
	while(1) {
		/* TODO: Take the mutex */
		pthread_mutex_lock(&lock);
		
		/* TODO: Wait if there is no client to be served. */
		while (numfull == 0) {
		  pthread_cond_wait(&fill, &lock);
		}
			
		/* TODO: Get the dispatch time */
		gettimeofday(&dispatch, NULL);
		
		/* TODO: Set the ID of the the thread in charge */
		threadid = worker.id;
		
		/* Get the request from the queue according to the sched algorithm */
		if (algorithm == FIFO) {
			/* TODO: FIFO=Removes the first request in the queue */
			
		} else if (algorithm == SFF) {
			/* TODO: SFF=Removes the request with the smalles file first */
		}

		/* TODO: Set the dispatch time of the request */
		req->dispatch = calculate_time(dispatch);
		
		/* TODO: Signal that there is one request left */
		printf("There is only one request left!");
		
		/* Update Server statistics */
		clients_treated++;
		latencies_acc += (long)(req->dispatch - req->arrival);

		/* TODO: Synchronize */

		/* TODO: Dispatch the request to the Request module */

		printf("Latency for client %d was %ld\n", worker.client_id, (long)(req->dispatch - req->arrival));
		printf("Avg. client latency: %.2f\n", (float)latencies_acc/(float)clients_treated);

		/* TODO: Close connection with the client */
	}
}

int main(int argc, char *argv[])
{
	/* Variables for the connection */
	int listenfd, connfd, clientlen; 
	struct sockaddr_in clientaddr;
	
	/* Variables for the user arguments */
	int port;
	int threads, buffers;
	sched_alg alg;

	/* Timestamp variables */
	struct timeval arrival;

	/* Parse the input arguments */
	getargs(argc, argv, &port, &threads, &buffers, &alg);

	/*  TODO:
	 *  Initialize the global variables:
	 *     max,
	 *     buffers,
	 *     numfull,
	 *     fillptr,
	 *     useptr,
	 *     algorithm  */
	max = threads;
	//buffers = buffers;
	numfull = 0;
	fillptr = 0;
	useptr = 0;
	algorithm = alg;
	
	/* TODO: Allocate the requests queue */
	/* done */
	buffer = malloc(max * sizeof(request*));
	
	/* TODO: Allocate the threads buffer */
	/* done */
	pthread_t thread_buffer[threads];
	
	int i;
	int status;
	for(i = 0; i < threads; i++) {
	  /* TODO: Create N consumer threads */
	  /* done */
	  status = pthread_create(&thread_buffer[i], NULL, consumer, (void *)&i);
	  if(status) {
	    printf("ERROR; return code from pthread_create() is %d\n", status);
	    exit(-1);
	  }
	}

	/* Main Server Loop */
	listenfd = Open_listenfd(port);
	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

		/* Save the arrival timestamp */
		gettimeofday(&arrival, NULL);

		/* TODO: Take the mutex to modify the requests queue */
		pthread_mutex_lock(&lock);
		
		/* TODO: If the request queue is full, wait until somebody frees one slot */
		while (numfull == max) {
		  pthread_cond_wait(&empty, &lock);
		}
		
		/* Allocate a request structure */
		request *req = malloc(sizeof(request)); 

		/* TODO: Fill the request structure */
		req->fd = listenfd;
		req->size = clientlen;
		req->arrival = calculate_time(arrival);
		
		/* Queue new request depending on scheduling algorithm */
		if (alg == FIFO) {
			/* TODO: FIFO=Queue request at the end of the queue */
		} else if(alg == SFF) {
			/* TODO: SFF=Queue request sorting them according to file size */
			/* HINT: 
			   You can use requestFileSize() to check the size of the file requested.
			   You can use qsort() with requestcmp() to sort the requests by size.
			 */
		}

		/* TODO: Increase the number of clients queued */
		numfull = numfull + 1;
		
		/* TODO: Synchronize */
	}
}


    


 
