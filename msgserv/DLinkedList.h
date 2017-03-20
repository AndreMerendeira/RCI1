/*
 * DLinkedList.h
 *
 *  Created on: 20/03/2017
 *      Author: andre
 */

#ifndef DLINKEDLIST_H_
#define DLINKEDLIST_H_

typedef struct Node Node;

struct Node {
   char msg[140];
   int key;

   struct Node *next;
   struct Node *prev;
};

void insertSorted(int newKey, char msg[140], Node* first, Node* last);
void removeLast(Node* first, Node* last);

#endif /* DLINKEDLIST_H_ */
