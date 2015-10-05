#ifndef __SERVER_TYPES__
#define __SERVER_TYPES__

/* Thread structure */
typedef struct {
	int id;
	int count;
	int statics;
	int dynamics;
	int client_id;
} thread;


/* Supported scheduling algorithms */
typedef enum sched_alg_s {
	FIFO,
	STACK,
	SFF,
	BF,
} sched_alg;

#endif
