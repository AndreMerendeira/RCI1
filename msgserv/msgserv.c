/*
 * msgserv.c
 *
 *  Created on: 24/02/2017
 *      Author: andre
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "msgserv.h"
#include "DLinkedList.h"

#define max(A,B) ((A)>=(B)?(A):(B))


int main (int argc, char** argv) {
	int i, fd, new_fd, id_socket = -1, max_fd, afd, counter, should_exit = 0;
	char* s_name = NULL, *upt = NULL, *tpt = NULL;
	char line[50], command[50], ip[20];
	struct sockaddr_in id_addr ;
	enum {idle,busy} state;
	uint16_t  sipt = 0;
	struct hostent* id_serv;
	struct timeval refresh;
	int m = 200, r = 10;
	fd_set readfds;

	memset((void*)&id_addr, (int)'\0', sizeof(id_addr));

	//Default value for optional parameter -i
	if((id_serv = gethostbyname("tejo.tecnico.ulisboa.pt"))==NULL)
		exit(1);//error
	else
		id_addr.sin_addr.s_addr = ((struct in_addr*) (id_serv->h_addr_list[0]))->s_addr;

	sipt = htons((ushort)59000);

	//Command parsing
	for (i=1; i<argc; i++) {
		if (strcmp( argv[i], "-n") == 0)
			s_name = argv[++i];

		else if (strcmp(argv[i],"-j") == 0) {
			if (inet_aton(argv[++i], (struct in_addr*) NULL) == 0) {
				printf("IP address not valid! ('-j' parameter)\n");
				exit(1);
			}
			strcpy(ip, argv[i]);
		}
		else if (strcmp(argv[i],"-u") == 0)
			upt = argv[++i];

		else if (strcmp(argv[i],"-t") == 0)
			tpt = argv[++i];

		else if (strcmp(argv[i],"-i") == 0 ) {
			if (inet_aton(argv[++i], &id_addr.sin_addr) == 0) { //REVIEW
				printf("IP address not valid! ('-i' optional parameter)\n");
				exit(1);
			}
		}
		else if (strcmp(argv[i],"-p") == 0)
			sipt = htons((ushort)atoi(argv[++i]));

		else if (strcmp(argv[i],"-m") == 0)
			m = atoi(argv[++i]);

		else if (strcmp(argv[i],"-r") == 0)
			r = atoi(argv[++i]);
	}

	//If any of the required parameters hasn't been set, then report error
	if (s_name == NULL || ip == 0 || upt == NULL || tpt == NULL ) {
		printf("'-n' or '-j' or '-u' or '-t' parameter(s) missing!\n");
		exit(1);
	}

	//"Fill" siip_addr
	id_addr.sin_family = AF_INET;
	id_addr.sin_port = sipt;

	//Set REG refresh rate
	refresh.tv_sec = r;
	refresh.tv_usec = 0;

	//Create UDP server at port upt to receive requests from terminals

	//------TO BE DONE--------


	//Create TCP server at port tpt to receive session start requests from other msg servers

	//------TO BE DONE--------


	state = idle;
	//Read commands from STDIO
	while (!should_exit){
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);
		max_fd = fd;
		if(state == busy){
			FD_SET(afd, &readfds);
			max_fd = max(max_fd, afd);
		}

		//if the client is connected, then it must "look" to STDIO and all connections
		if(id_socket != -1) {
			//look to all connections and STDIO
			FD_SET(id_socket, &readfds);
			counter = select(max_fd+1, &readfds, (fd_set*)NULL,(fd_set*)NULL, &refresh);

			//Re-arm refresh timer
			refresh.tv_sec = r;
			refresh.tv_usec = 0;

			if(counter < 0) {
				printf("Error in select\n");
				exit(1);//error
			}
			//Periodically register with ID server
			if (counter == 0)
				id_socket = join_id(id_socket, id_addr, s_name, ip, upt, tpt);

		}
		//if not, it only receives inputs from STDIO
		else
			select(1, &readfds, (fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);

		//Input from STDIO
		if(FD_ISSET(0, &readfds)){
			fgets (line, 100, stdin);
			if (sscanf (line, "%s", command) == 1){

				// join command
				if (strcmp (command, "join") == 0){
					if (id_socket !=-1)
						printf("Already connected as %s, please exit and re-join if you want to be a different server\n", s_name);
					else {
						id_socket = join_id(id_socket, id_addr, s_name, ip, upt, tpt);
						if (id_socket == -1)
							printf("Could not connect to identities server\n");
					}
				}

				// exit command
				else if (strcmp (command, "exit") == 0){
					printf("Server exiting...\n");
					if (id_socket == -1)
						should_exit = 1;
					else {
						if (disconnect(id_socket) != -1)
							should_exit = 1;
						else
							printf("Couldn't exit");
					}
				}
				/*
				// show_servers command
				else if ( strcmp( command, "show_servers") == 0) {
					if (id_socket == -1)
						printf("Please connect to identities server using join\n");
					else
						show_servers (put needed arguments here;
				}
				 */

				/*
			// show_messages command
			else if ( strcmp( command, "show_messages") == 0) {
				if (id_socket == -1)
					printf("Please connect to identities server using join\n");
				else
					show_messages(put needed arguments here)
			}
				 */
				else
					printf("Invalid command\n");
			}
			else
				printf("Error in command\n");
		}
	}

	exit(0);
}


int join_id (int id_socket, struct sockaddr_in id_addr, char* s_name, char* ip, char* upt, char* tpt) {

	int fd, ret;
	char* buffer;
	struct sockaddr_in addr;

	memset((void*)&addr, (int)'\0', sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port = htons((ushort)atoi(upt));

	//Create UDP socket if needed
	if (id_socket == -1) {
		if((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
			printf("Error creating a socket to connect to ID server\n");
			exit(1);//error
		}
		//Bind socket ID server
		ret = bind(fd, (struct sockaddr*) &addr, sizeof(addr));
		if(ret == -1) {
			printf("Error binding ID server address\n");
			exit(1);//error
		}
	}

	//Register at the ID server (send REG msg)
	buffer = (char*)malloc(sizeof(char)*(7+strlen(s_name)+strlen(ip)+strlen(upt)+strlen(tpt)));
	sprintf(buffer, "REG %s;%s;%s;%s", s_name, ip, upt, tpt);
	if (sendto(fd, buffer, strlen(buffer), 0, (struct sockaddr*) &id_addr, sizeof(id_addr)) == -1) {
		printf("Error sending REG msg to ID server\n");
		exit(1);
	}

	free(buffer);

	return fd;
}

int disconnect(int id_socket) {
	return (close(id_socket));
}



