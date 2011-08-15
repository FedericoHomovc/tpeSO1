/***
***
***		fifo.h
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/

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
