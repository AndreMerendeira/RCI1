/*
 * msgserv.h
 *
 *  Created on: 06/03/2017
 *      Author: andre
 */

#ifndef MSGSERV_H_
#define MSGSERV_H_

<<<<<<< HEAD
#include "DLinkedList.h"

int join_id  (int id_socket, struct sockaddr_in id_addr, char* s_name, char* ip, char* upt, char* tpt);
int disconnect(int id_socket);
void show_messages (Node* last);
char* get_n_messages (Node* first, int n, int stored);
=======
int join_id  (int id_socket, struct sockaddr_in id_addr, char* s_name, char* ip, char* upt, char* tpt);
int disconnect(int id_socket);


>>>>>>> refs/heads/server

#endif /* MSGSERV_H_ */
