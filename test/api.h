#ifndef API_H_
#define API_H_

/*
 *
 * Includes
 * 
 */

#include <sys/types.h>

/*
 *
 * Symbolic constants
 *
 */

#define TRUE 1
#define FALSE 0

/* 
 *
 * Structs
 *
 */

typedef struct serverCDT * servADT;
typedef struct IPCCDT * comuADT;


typedef struct IPCMessage
{
	long size;
	void * message;
} message;

typedef struct
{
	pid_t id;
	comuADT comm;
} infoClient;



/*
 * Functions
 */

servADT startServer();

comuADT connectToServer(servADT serv);

comuADT getClient(servADT serv, pid_t id);

int sendMsg(comuADT comm, message * msg, int flags);

int rcvMsg(comuADT comm, message * msg, int flags);

int disconnectFromServer(comuADT comm, servADT server);

int endServer(servADT server);

int infoClient_comparePid(infoClient * ic1, infoClient * ic2);

#endif /* API_H_ */
