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
  //assert(*(request**)first != NULL);
  if((*(request**)first == NULL) || (*(request**)second == NULL)) {
    return 0;
  }
  //printf("first-size: %ld\n", (*(request**)first)->size);
  printf("compare results in: %ld\n",(*(request**)first)->size - (*(request **)second)->size);
  return (*(request**)first)->size - (*(request **)second)->size;
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
	worker.id = *(int*)arg;
	worker.count = 0;
	worker.statics = 0;
	worker.dynamics = 0;
	worker.client_id = 0;

	volatile request *req = NULL;
	struct timeval dispatch;

	/* Main thread loop */
	while(1) {
		/* TODO: Take the mutex */
		pthread_mutex_lock(&lock);
		
		/* TODO: Wait if there is no client to be served. */
		while (numfull == 0) {
		  printf("Waiting for requests\n");
		  pthread_cond_wait(&fill, &lock);
		}
		
		/* TODO: Get the dispatch time */
		gettimeofday(&dispatch, NULL);
		
		/* TODO: Set the ID of the the thread in charge */
		threadid = worker.id;
		
		/* Get the request from the queue according to the sched algorithm */
		if (algorithm == FIFO) {
		  req = *(buffer + 0);
		  //numfull = numfull - 1;
		  *(buffer + 0) = NULL;
		  int i = 0;
		  //while ((*(buffer + i + 1) != NULL) && (i < max-1) && max != 1)  {
		  while ((*(buffer + i + 1) != NULL) && (i < numfull - 1) && numfull != 1)  {
		    printf("Moving queue[%d] to queue[%d]\n", i+1, i);
		    *(buffer + i) = *(buffer + i + 1);
		    *(buffer + i + 1) = NULL;
		    i = i + 1;
		  }
		  numfull = numfull - 1;
		  pthread_cond_signal(&empty);
		} else if (algorithm == SFF) {
		  /* TODO: SFF=Removes the request with the smalles file first */
		  /* int i; */
		  /* long min = requestFileSize((*(buffer + 0))->fd); */
		  /* for (i = 1; i < max; i++) { */
		  /*   long temp = requestFileSize((*(buffer + i))->fd); */
		  /*   if (temp < min) { */
		  /*     min = temp; */
		  /*   } */
		  /* } */
		  /* for (i = 0; i < max; i++) { */
		  /*   long temp = requestFileSize((*(buffer + i))->fd); */
		  /*   if (temp == min) { */
		  /*     req = *(buffer + i); */
		  /*     int j = i; */
		  /*     while ((*(buffer + j + 1) != NULL) && (j < max-1) && max != 1) { */
		  /* 	//while (*(buffer + j + 1)) { */
                  /*       *(buffer + j) = *(buffer + j + 1); */
                  /*       *(buffer + j + 1) = NULL; */
                  /*       j = j + 1; */
		  /*     } */
		  /*   } */
		  /* } */
		  req = *(buffer + 0);
		  //numfull = numfull - 1;
		  *(buffer + 0) = NULL;		  
		  int j = 0;
		  //while ((*(buffer + j + 1) != NULL) && (j < max-1) && max != 1) {
		  while ((*(buffer + j + 1) != NULL) && (j < numfull - 1) && numfull != 1) {
		    //while (*(buffer + j + 1)) {
		    printf("curr j: %d\n",j);
		    *(buffer + j) = *(buffer + j + 1);
		    *(buffer + j + 1) = NULL;
		    j = j + 1;
		  }	
		  numfull = numfull - 1;
		  pthread_cond_signal(&empty);
		}

		/* TODO: Set the dispatch time of the request */
		req->dispatch = calculate_time(dispatch);
		
		/* Update Server statistics */
		clients_treated++;
		latencies_acc += (long)(req->dispatch - req->arrival);

		/* TODO: Synchronize */
		 pthread_mutex_unlock(&lock);
        
		/* TODO: Dispatch the request to the Request module */
		requestHandle(req->fd,req->arrival,req->dispatch, &worker);
    
		printf("Latency for client %d was %ld\n", worker.client_id, (long)(req->dispatch - req->arrival));
		printf("Avg. client latency: %.2f\n", (float)latencies_acc/(float)clients_treated);

		/* TODO: Close connection with the client */
		Close(req->fd);
		req = NULL;
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
	for(int i=0; i < max; i++) {
	  *(buffer + i) = NULL;
	}
	
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
		/* printf("max: %d\n", max); */
		/* printf("numfull: %d\n", numfull); */
		while (numfull == max) {
		  printf("QUEUE FULL: %d/%d\n", numfull, max);
		  pthread_cond_wait(&empty, &lock);
		  printf("SPACE AVAILABLE IN QUEUE AGAIN\n");
		}
		
		/* Allocate a request structure */
		request *req = malloc(sizeof(request)); 

		/* TODO: Fill the request structure */
		req->fd = connfd;
		req->size = requestFileSize(req->fd);
		req->arrival = calculate_time(arrival);
		
		/* Queue new request depending on scheduling algorithm */
		if (alg == FIFO) {
		  // TODO: Add the request to the buffer properly
		  // *buffer = req;
		  int i = 0;
		  while ((*(buffer + i) != NULL) && i < max) {
		    printf("Index %d in buffer taken, try next\n", i);
		    i = i + 1;
		  }
		  *(buffer + i) = req;
		  //numfull = numfull + 1;
		  // This signals to the threads that there is a new request in queue
		  pthread_cond_signal(&fill);
		  /* TODO: FIFO=Queue request at the end of the queue */
            
		  // NOTE: I do not know what you mean exactly, because we have an array
		  // which automatically means that we are adding a single request at
		  // the end, always.
            
		} else if(alg == SFF) {
		  /* TODO: SFF=Queue request sorting them according to file size */
		  int i = 0;
		  while ((*(buffer + i) != NULL) && i < max) {
		    printf("Size of q[%d]: %ld\n", i, (*(buffer + i))->size);		    
		    i = i + 1;
		  }
		  *(buffer + i) = req;

		  printf("------------- BEFORE SORTING ------------\n");
		  //for(int x = 0; x < max; x++) {
		  for(int x = 0; x < numfull; x++) {
		    if(*(buffer+x) == NULL) {
		      printf("buffer[%d]: NULL\n", x);
		    } else {
		      printf("buffer[%d]: %ld\n", x, (*(buffer+x))->size);
		    }
		  }
		  
		  //qsort(buffer, max, sizeof(request*), requestcmp);	
		  qsort(buffer, numfull, sizeof(request*), requestcmp);	
		  
		  printf("!!!!!!!!!!!!! AFTER SORTING !!!!!!!!!!!!!!\n");
		  //for(int x = 0; x < max; x++) {
		  for(int x = 0; x < numfull; x++) {
		    if(*(buffer+x) == NULL) {
		      printf("buffer[%d]: NULL\n", x);
		    } else {
		      printf("buffer[%d]: %ld\n", x, (*(buffer+x))->size);
		    }
		  }		  

		  pthread_cond_signal(&fill);
		}
		
		/* TODO: Increase the number of clients queued */
		numfull = numfull + 1;
		printf("Added req in queue. Queued requests: %d\n", numfull);
		
		/* TODO: Synchronize */
		pthread_mutex_unlock(&lock);
	}
}
