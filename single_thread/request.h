#ifndef __REQUEST_H__

long requestFileSize(int fd);

void requestHandle(int fd, long arrival, long dispatch);

#endif
