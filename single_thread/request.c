/**
 * request.c: Does the bulk of the work for the web server.
 *
 * Course: 1DT032
 * 
 */

#include <assert.h>
#include "util.h"
#include "request.h"

/**
 * requestError
 *
 * ARGUMENTS:
 * @fd:
 * @cause:
 * @errnum:
 * @shortmsg:
 * @longmsg:
 */
void requestError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
	assert(cause != NULL);
	assert(errnum != NULL);
	assert(shortmsg != NULL);
	assert(longmsg != NULL);

	char buf[MAXLINE], body[MAXBUF];

	printf("Request ERROR\n");

	/* Create the body of the error message */
	sprintf(body, "<html><title>1DT032 Error</title>");
	sprintf(body, "%s<body bgcolor=""fffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr>1DT032 Web Server\r\n", body);

	/* Write out the header information for this response */
	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	Rio_writen(fd, buf, strlen(buf));
	printf("%s", buf);

	sprintf(buf, "Content-Type: text/html\r\n");
	Rio_writen(fd, buf, strlen(buf));
	printf("%s", buf);

	sprintf(buf, "Content-Length: %lu\r\n\r\n", strlen(body));
	Rio_writen(fd, buf, strlen(buf));
	printf("%s", buf);

	/* Write out the content */
	Rio_writen(fd, body, strlen(body));
	printf("%s", body);
}


/**
 * Reads and discards everything up to an empty text line
 */
void requestReadHdrs(rio_t *rp) {
    /* 
     * TODO:
     * NOTE: Probably done, I may have misunderstood.
     * The previous line will only read one line, however, it should
     * discard everything up to an empty text line.
     */
	assert(rp != NULL);

	char buf[MAXLINE];
	Rio_readlineb(rp, buf, MAXLINE);
	while(strcmp(buf,"\r\n") != 0) {
	  printf("%s\n", buf);
	  Rio_readlineb(rp, buf, MAXLINE);
	}

	return;
}

/**
 * Parses the request URI.
 * Calculates @filename (and @cgiargs for dynamic) from @uri.
 *
 * RETURNS: 1 if static, 0 if dynamic content.
 */
int requestParseURI(char *uri, char *filename, char *cgiargs) {
	assert(uri != NULL);
	assert(filename != NULL);
	assert(cgiargs != NULL);

	char *ptr;

	if (!strstr(uri, "cgi")) {
		/* Static */
		strcpy(cgiargs, "");
		sprintf(filename, ".%s", uri);
		if (uri[strlen(uri)-1] == '/') {
			strcat(filename, "home.html");
		}
		return 1;
	} else {
		/* Dynamic */
		ptr = index(uri, '?');
		if (ptr) {
			strcpy(cgiargs, ptr+1);
			*ptr = '\0';
		} else {
			strcpy(cgiargs, "");
		}
		sprintf(filename, ".%s", uri);
		return 0;
	}
}

/**
 * Fills in the @filetype given the @filename.
 */
void requestGetFiletype(char *filename, char *filetype) {
	assert(filename != NULL);
	assert(filetype != NULL);

	if (strstr(filename, ".html")) {
		strcpy(filetype, "text/html");
	} else if (strstr(filename, ".gif")) {
		strcpy(filetype, "image/gif");
	} else if (strstr(filename, ".jpg")) {
		strcpy(filetype, "image/jpeg");
	} else { 
		strcpy(filetype, "test/plain");
	}
}

/**
 * Handles a dynamic request.
 */
void requestServeDynamic(int fd, char *filename, char *cgiargs, long arrival, long dispatch) {
	assert(filename != NULL);
	assert(cgiargs != NULL);

	char buf[MAXLINE], *emptylist[] = {NULL};

	/* The server does only a little bit of the header.
	 * The CGI script has to finish writing out the header. */
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: 1DT032 Web Server\r\n", buf);
    sprintf(buf, "%s Stat-req-arrival: %ld\r\n", buf, arrival);
	sprintf(buf, "%s Stat-req-dispatch: %d\r\n", buf, (int)(dispatch - arrival));

	Rio_writen(fd, buf, strlen(buf));

	if (Fork() == 0) {
		/* Child process */
		Setenv("QUERY_STRING", cgiargs, 1);
		/* When the CGI process writes to stdout, it will instead go to the socket */
		Dup2(fd, STDOUT_FILENO);
		Execve(filename, emptylist, environ);
	}
	Wait(NULL);
}


/**
 * Handles a static request.
 * Responds to @fd, sending @filesize bytes from @filename.
 */
void requestServeStatic(int fd, char *filename, int filesize, long arrival, long dispatch) {
	assert(filename != NULL);

	int srcfd;
	char *srcp, filetype[MAXLINE], buf[MAXBUF];
	struct timeval beforeread, afterread;
	int read, complete;

	gettimeofday(&beforeread, NULL);

	requestGetFiletype(filename, filetype);

	srcfd = Open(filename, O_RDONLY, 0);

	/* Rather than call read() to read the file into memory, 
	   which would require that we allocate a buffer, we memory-map the file */
	srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	Close(srcfd);

	gettimeofday(&afterread, NULL);

	read = ((afterread.tv_sec - beforeread.tv_sec) * 1000 + (afterread.tv_usec - beforeread.tv_usec)/1000.0) + 0.5;
	complete = (((afterread.tv_sec) * 1000 + (afterread.tv_usec)/1000.0) + 0.5) - arrival;

	/* Put together response */
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: 1DT032 Web Server\r\n", buf);
	sprintf(buf, "%s Stat-req-arrival: %ld\r\n", buf, arrival);
	sprintf(buf, "%s Stat-req-dispatch: %d\r\n", buf, (int)(dispatch - arrival));
	sprintf(buf, "%s Stat-req-read: %d\r\n", buf, read);
	sprintf(buf, "%s Stat-req-complete: %d\r\n", buf, complete); 

	sprintf(buf, "%sContent-Length: %d\r\n", buf, filesize);
	sprintf(buf, "%sContent-Type: %s\r\n\r\n", buf, filetype);

	Rio_writen(fd, buf, strlen(buf));

	/*  Writes out to the client socket the memory-mapped file */
	Rio_writen(fd, srcp, filesize);
	Munmap(srcp, filesize);

}


/**
 * Receives and handles a request from @fd, setting the
 * @arrival and @dispatch times for the statistics.
 */
void requestHandle(int fd, long arrival, long dispatch) {
	int is_static;
	struct stat sbuf;
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];
	rio_t rio;

	Rio_readinitb(&rio, fd);
	Rio_readlineb(&rio, buf, MAXLINE);
	
	sscanf(buf, "%s %s %s", method, uri, version);

	printf("%s %s %s\n", method, uri, version);

	/* TODO: Return an error if a GET method is received. */
    if (strcmp(method,"GET") != 0) {
        requestError(fd,filename,"501","Not implemented","1DT032 Server either does not recognize the request method, or it lacks the ability to fulfill the request");
    }
	/* Read request headers */
	requestReadHdrs(&rio);

	/* Parse the URI and check for static request */
	is_static = requestParseURI(uri, filename, cgiargs);
	
	/* Check if file is present in the directory */
	if (stat(filename, &sbuf) < 0) {
		/* The file requested is missing. Produce an error */
		requestError(fd, filename, "404", "Not found", 
			     "1DT032 Server could not find this file");
		return;
	}

	if (is_static) {
		/* Static request */
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
			requestError(fd, filename, "403", "Forbidden", "1DT032 Server could not read this file");
			return;
		}
		/* Delegate the request processing to the Request module */
		requestServeStatic(fd, filename, sbuf.st_size, arrival, dispatch);
	} else {
		/* TODO: Implement the dynamic case */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            requestError(fd, filename, "403", "Forbidden", "1DT032 Server could not read this file");
            return;
        }
		/* Delegate the request processing to the Request module */
		requestServeDynamic(fd, filename, cgiargs, arrival, dispatch);
	}
}


/**
 * Returns the total size of a request.
 */
long requestFileSize(int fd) {
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];	
	struct stat s;

	/* Receive some bytes from the client */
	recv(fd, buf, sizeof(buf), MSG_PEEK);
	
	/* Parse method, uri and version from buf */
	sscanf(buf, "%s %s %s\n", method, uri, version);

	/* Parse URI */
	requestParseURI(uri, filename, cgiargs);

	Stat(filename, &s);

	return (long)s.st_size;
}


