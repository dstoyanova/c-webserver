#ifndef __REQUEST_H__

#include "server_types.h"

long requestFileSize(int fd);

/* TODO: Add a parameter to assign a worker to each request */
/* done */
void requestHandle(int fd, long arrival, long dispatch, struct thread& worker);

#endif
