/*
 * rmb.h
 *
 *  Created on: 07/03/2017
 *      Author: andre
 */

#ifndef RMB_H_
#define RMB_H_

int disconnect(int id_socket);

void show_servers (int id_socket, struct sockaddr_in addr);
int publish(int id_socket, struct sockaddr_in id_addr, char* message, int s_socket, struct sockaddr_in* msg_serv_addr);
int connect_to_s (int id_socket, struct sockaddr_in id_addr, int s_socket, struct sockaddr_in* msg_serv_addr) ;

#endif /* RMB_H_ */
