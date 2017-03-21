/*
 * rmb.c
 *
 *  Created on: 07/03/2017
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

#include "rmb.h"

int main (int argc, char** argv) {
	int i, id_socket = -1, s_socket = -1, should_exit = 0, ret;
	char line[50], command[50], message[140];
	struct sockaddr_in id_addr, *msg_serv_addr = NULL;
	uint16_t  sipt = 0;
	struct hostent* id_serv;
	fd_set readfds;

	memset((void*)&id_addr, (int)'\0', sizeof(id_addr));

	//Default value for optional parameter -i
	if((id_serv = gethostbyname("tejo.tecnico.ulisboa.pt")) == NULL)
		exit(1);//error
	else
		id_addr.sin_addr.s_addr = ((struct in_addr*) (id_serv->h_addr_list[0]))->s_addr;

	sipt = htons((ushort)59000);

	//Command parsing
	for (i=1; i<argc; i++) {
		if (strcmp(argv[i],"-i") == 0 ) {
			if (inet_aton(argv[++i], &id_addr.sin_addr) == 0) {
				printf("IP address not valid! ('-i' optional parameter)\n");
				exit(1);
			}
		}
		else if (strcmp(argv[i],"-p") == 0)
			sipt = htons((ushort)atoi(argv[++i]));
	}

	//"Fill" id_addr
	id_addr.sin_family = AF_INET;
	id_addr.sin_port = sipt;

	//Create ID server socket
	if((id_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		printf("Error creating a socket to connect to ID server\n");
		exit(1);//error
	}

	//Read commands from STDIO
	while (!should_exit){
		fgets (line, 100, stdin);
		if (sscanf (line, "%s", command) == 1){

			// show_servers command
			if (strcmp (command, "show_servers") == 0)
				show_servers(id_socket, id_addr);

			// publish command
			else if (strcmp (command, "publish") == 0) {
				if(sscanf(line, "%*s %[^\n]", message) == 1)
					publish(id_socket, id_addr, message, s_socket ,msg_serv_addr);
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
			else
				printf("Invalid command\n");
		}
		else
			printf("Error in command\n");
	}

	exit(0);
}

void show_servers (int id_socket, struct sockaddr_in addr) {
	int addrlen;
	char* buffer;
	char response[300];

	buffer = (char*)malloc(sizeof(char)*strlen("GET_SERVERS"));
	sprintf(buffer, "GET_SERVERS");

	addrlen=sizeof(addr);

	//Send msg to ID server
	if (sendto(id_socket, buffer, strlen(buffer), 0, (struct sockaddr*) &addr, addrlen) == -1) {
		printf("Error sending GET_SERVERS msg to ID server\n");
		exit(1);
	}

	//Receive response
	if (recvfrom(id_socket, response, 300, 0, (struct sockaddr*) &addr,  (socklen_t*)&addrlen) == -1) {
		printf("Error receiving list of servers from ID server\n");
		exit(1);
	}
	printf("%s", response);

	free(buffer);

	return;
}


int publish(int id_socket, struct sockaddr_in id_addr, char* message, int s_socket, struct sockaddr_in* msg_serv_addr) {
	int new_socket;

	//Not yet connected to a server
	if (s_socket == -1)  {
		msg_serv_addr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
		memset((void*)msg_serv_addr, (int)'\0', sizeof(struct sockaddr_in ));

		if((new_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
			printf("Error creating a socket to connect to message server\n");
			return(-1);//error
		}
		if( connect_to_s(id_socket,id_addr, s_socket, msg_serv_addr) == -1)
			return -1;

		if (sendto(s_socket, (void*) message, strlen(message), 0, (struct sockaddr*) &id_addr, sizeof(id_addr)) == -1) {
			printf("Error sending msg to message server\n");
			return(-1);
		}
		else
			return new_socket;
	}
	//Already connected to a server
	else {

	}


	return s_socket;
}


int disconnect(int id_socket) {
	return (close(id_socket));
}

int connect_to_s (int id_socket, struct sockaddr_in id_addr, int s_socket, struct sockaddr_in* msg_serv_addr) {
	int addrlen, upt, s_active = 0, ret;
	char* buffer, *line;
	char response[300], *response_aux, *ip;
	fd_set readfds;
	struct timeval timeout;

	buffer = (char*)malloc(sizeof(char)*strlen("GET_SERVERS"));
	sprintf(buffer, "GET_SERVERS");

	addrlen = sizeof(id_addr);

	//Send msg to ID server
	if (sendto(id_socket, buffer, strlen(buffer), 0, (struct sockaddr*) &id_addr, addrlen) == -1) {
		printf("Error sending GET_SERVERS msg to ID server\n");
		return(-1);
	}

	//Receive response
	if (recvfrom(id_socket, response, 300, 0, (struct sockaddr*) &id_addr,  (socklen_t*)&addrlen) == -1) {
		printf("Error receiving list of servers from ID server\n");
		return(-1);
	}

	response_aux = strdup(response);
	//Ignore first line
	strsep(&response_aux, "\n");

	while (!s_active) {
		line = strsep(&response_aux, "\n");
		if(strcmp(line, "\0") == 0) {
			printf("No servers registered on ID server!\n");
			return (-1);
		}
		else {
			//Ignore name
			strdup( strsep(&line, ";") );
			//Get IP
			ip = strdup( strsep(&line, ";") );

			if(sscanf(line, "%d;%*d", &upt) == 1 && inet_aton(ip, &msg_serv_addr->sin_addr) != 0)  {
				msg_serv_addr->sin_port = htons((ushort)upt);
				msg_serv_addr->sin_family = AF_INET;

				timeout.tv_sec = 1;
				timeout.tv_usec = 0;

				ret = bind(s_socket, (struct sockaddr*) msg_serv_addr, sizeof( struct sockaddr_in));
				if(ret == -1) {
					printf("Error binding message server address\n");
					return(-1);//error
				}
				FD_ZERO(&readfds);
				FD_SET(s_socket, &readfds);
				if(select(s_socket, &readfds, (fd_set*)NULL,(fd_set*)NULL, &timeout) == 1)
					s_active = 1;
			}
		}
	}
	return 1;
}



