#include "util.h"

int main() {
  char buf[MAXLINE];

  rio_t rp;
  int len = Rio_readlineb(&rp, buf, MAXLINE);
  printf("test: %d\n", len);
  /* while(len != 0) { */
  /*   rp->rio_bufptr += len; */
  /*   Rio_readlineb(rp, buf, MAXLINE); */
  /* } */
   
}
