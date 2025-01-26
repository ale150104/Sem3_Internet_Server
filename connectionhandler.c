#include <errno.h>
#include "connectionhandler.h"
#include "util.h"
#include "Cookie.h"
#include "Clientthread.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int createPassiveSocket(in_port_t port)
{
	int fd = -1;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1){
		errno = ENOSYS;
		return fd;
	}

	struct sockaddr_in address;
	memset(&address, 0, (sizeof(address)));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	if(bind(fd, (const struct sockaddr *)&address, sizeof(address)) < 0){
		errorPrint("Beim Binden an das Socket ist etwas schief gelaufen");
		return -1;
	} 

	infoPrint("An Socket binden hat funktioniert");

	if(listen(fd, 3) < 0){
		errorPrint("Beim Lauschen auf das Socket ist etwas schiefgelaufen");
		return -1;
	} 
	infoPrint("Auf Socket lauschen hat funktioniert");

	return fd;
}

int connectionHandler(in_port_t port)
{
	const int fd = createPassiveSocket(port);
	if(fd == -1)
	{
		errnoPrint("Unable to create server socket");
		return -1;
	}

	for(;;)
	{
		int sd = accept(fd, NULL, NULL);
		infoPrint("Neue Verbindungsanfrage gerade rein gekommen");

		if(sd < 0){
			errorPrint("Erstellung eines Sockets konnte nicht ");
		}

		normalPrint("Neuer Socket für Client-Verbindung hergestellt");

  

		//New Thread for Client Request
		pthread_t clientThread;
		int result = pthread_create(&clientThread, NULL, clientthread, &sd);
		if(result != 0){
			errorPrint("Erzeugen des Client-Threads schief gelaufen");
		} 

	    infoPrint("Thread angelegt.");
		 
	}

	return 0;	//never reached
}
