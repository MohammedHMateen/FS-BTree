#include <stdlib.h>

#include "stack.h"
#include "btree.h"

#ifndef STACK_SIZE

/* Linked Stack */
typedef struct _stack {
	type_node* node;
	struct _stack* prev;
} stack;

stack* tail = NULL;

void pushNode(type_node* node) {
	stack* newTail = (stack*)malloc(sizeof(stack));
	newTail->node = node;
	newTail->prev = tail;
	tail = newTail;
}

type_node* popNode() {
	if(tail == NULL) return NULL;

	type_node* node = tail->node;
	stack* newTail = tail->prev;
	free(tail);
	tail = newTail;
	return node;
}

void clearNodes() {
	stack* curr = tail;
	while(curr != NULL) {
		tail = curr->prev;
		free(curr);
		curr = tail;
	}
}

#else
type_node* stack[STACK_SIZE];
int stackCount = 0;

void pushNode(type_node* node) {
#ifdef CHECK_STACK_OVERFLOW
	if(stackCount + 1 > STACK_SIZE) {
		printf("STACK OVERFLOW");
		exit(-1);
	}
#endif
	stack[stackCount++] = node;
}

type_node* popNode() {
	if(stackCount == 0) return NULL;
	return stack[--stackCount];
}

void clearNodes() {
	stackCount = 0;
}
#endif
