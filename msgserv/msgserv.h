/*
 * msgserv.h
 *
 *  Created on: 06/03/2017
 *      Author: andre
 */

#ifndef MSGSERV_H_
#define MSGSERV_H_

int join_id  (int id_socket, struct sockaddr_in id_addr, char* s_name, char* ip, char* upt, char* tpt);
int disconnect(int id_socket);



#endif /* MSGSERV_H_ */
