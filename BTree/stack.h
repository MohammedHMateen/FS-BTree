#ifndef STACK_H
#define STACK_H

#include "btree.h"

#define STACK_SIZE 1000
//#define CHECK_STACK_OVERFLOW

void pushNode(type_node* node);
type_node* popNode();
void clearNodes();

#endif
