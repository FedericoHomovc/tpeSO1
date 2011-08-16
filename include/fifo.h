/***
***
***		fifo.h
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/

/***		Module Defines		***/

#define	CLIENTS_SIZE	10
#define CHARS_ADD		10

#define SER_FIFO_S		"/tmp/serverFifo"
/*#define SER_FIFO_LEN	(strlen(SER_FIFO_S) + CHARS_ADD)*/
#define SER_FIFO_LEN	(25)

#define CLI_FIFO_R_S	"/tmp/clientFifo_r"
/*#define CLI_FIFO_R_LEN	(strlen(CLI_FIFO_R_S) + CHARS_ADD)*/
#define CLI_FIFO_R_LEN	(27)

#define CLI_FIFO_W_S	"/tmp/clientFifo_w"
/*#define CLI_FIFO_W_LEN	(strlen(CLI_FIFO_W_S) + CHARS_ADD)*/
#define CLI_FIFO_W_LEN	(27)

#define ERR_FIFO		-1

/*
 * Name: struct IPCCDT
 * Description: This struct is the implementation of comuADT for IPC via FIFOs.
 * Fields:
 * - clientFifo_r:	File descriptor for the FIFO used by a client to read.
 * - clientName:	Name of clientFifo_r in the file system.
 * - clientFifo_r:	File descriptor for the FIFO used by a client to write.
 * - clientName:	Name of clientFifo_w in the file system.
 */

struct IPCCDT
{
	int clientFifo_r;
	char clientName_r[CLI_FIFO_R_LEN];

	int clientFifo_w;
	char clientName_w[CLI_FIFO_W_LEN];
};


void itoa(int n, char *string);
void reverse(char *string);
int rcvMsg(comuADT comm, message *msg, int flags);
int sendMsg(comuADT comm, message *msg, int flags);
servADT startServer();
comuADT connectToServer(servADT serv);
comuADT getClient(servADT serv, pid_t id);
int disconnectFromServer(comuADT comm, servADT server);
int endServer(servADT server);
int infoClient_comparePid(infoClient * ic1, infoClient * ic2);
