#include <stdlib.h>
#include "connectionhandler.h"
#include "util.h"
#include <stdio.h>

int main(int argc, char **argv)
{

	utilInit(argv[0]);
	infoPrint("HTTP server, group LAyer");	//TODO: Add your group number!

	//TODO: evaluate command line arguments
	//TODO: perform initialization

	//TODO: use port specified via command line
	infoPrint("Übergebener Port %s", argv[1]);
	in_port_t port = 1025;
	
	if(argv[1] != NULL)
	{
		port = (in_port_t) atoi(argv[1]);
	} 

	infoPrint("Port-Wert: %d", port);

	const int result = connectionHandler((in_port_t)port);

	return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}




