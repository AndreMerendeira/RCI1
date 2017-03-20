/*
 * DLinkedList.c
 *
 *  Created on: 20/03/2017
 *      Author: andre
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "DLinkedList.h"

void insertSorted(int newKey, char msg[140], Node* first, Node* last) {
	//start from the first link
	Node *current = first, *new_node;

	new_node = (struct Node*) malloc(sizeof(struct Node));
	new_node->key = newKey;
	strcpy(new_node->msg, msg);

	//if list is empty
	if(first == NULL) {
		new_node->next = NULL;
		new_node->prev = NULL;
		first = new_node;
		last = new_node;
		return;
	}
	//else
	while( current->next != NULL && current->next->key > newKey) {
		//move to next link
		current = current->next;
	}
	if(current->next == NULL) {
		new_node->next = NULL;
		last = new_node;
	}
	else {
		new_node->next = current->next;
		current->next->prev = new_node;
	}

	new_node->prev = current;
	current->next = new_node;
	return;
}

void removeLast(Node* first, Node* last) {
	Node* aux_Node;

	//if only one link
	if(last->next == NULL) {
		free(last);
		last = NULL;
		first = NULL;
	}
	else {
		last->prev->next = NULL;
		aux_Node = last->prev;
		free (last);
	}
	last = aux_Node;

	return;
}


