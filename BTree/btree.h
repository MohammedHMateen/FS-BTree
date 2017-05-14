#ifndef BTREE_H
#define BTREE_H

#include <stdio.h>

#ifndef CAP_ORDER
#define CAP_ORDER 2
#endif

#define LEVEL(result) (result->one)
#define POSITION(result) (result->two)
#define LEFT_PARENT(result) (result->left)
#define RIGHT_PARENT(result) (result->right)

#define REDISTRIBUTIONS(result) (result->one)
#define MERGES(result) (result->two)

#define SPLITS(result) (result->one)


typedef struct _rec {
	char name[50];
	int leng;
} type_rec;

typedef struct _node {
	type_rec* records[CAP_ORDER * 2];
	struct _node* pointers[CAP_ORDER * 2 + 1];
	int count;
} type_node;

typedef struct _result {
	int one, two;
	type_rec *left, *right;
} type_result;

type_result* b_tree_insertation(char compName[50], type_node** root);
void b_tree_sequential_print(FILE* output, type_node* node);
type_result* b_tree_deletation(char searchName[50], type_node** root);
type_result* b_tree_search(char searchName[50], type_node* root);
void b_tree_free(type_node* node);

#endif
