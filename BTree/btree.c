#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btree.h"
#include "stack.h"

typedef struct _bignode {
	type_rec* records[CAP_ORDER * 2 + 1];
	struct _node* pointers[CAP_ORDER * 2 + 2];
	int count;
} type_bignode;

typedef struct _twobignode {
	type_rec* records[CAP_ORDER * 4 + 1];
	struct _node* pointers[CAP_ORDER * 4 + 2];
	int count;
} type_twobignode;

type_node* createNode() {
	type_node* node = (type_node*)malloc(sizeof(type_node));
	memset(node, 0, sizeof(type_node));
	return node;
}

type_rec* createRecord(char* compName) {
	type_rec* record = (type_rec*)malloc(sizeof(type_rec));
	strcpy(record->name, compName);
	record->leng = strlen(compName);
	return record;
}

type_result* createResult() {
	type_result* result = (type_result*)malloc(sizeof(type_result));
	memset(result, 0, sizeof(type_result));
	return result;
}

type_result* b_tree_insertation(char compName[50], type_node** root) {
	//char inKey[50];
	type_rec* inRecord;
	type_node *p, *curr, *newNode;
	type_result* res;
	int i, found, finished;
	int splits = 0;

	inRecord = createRecord(compName);

	if(*root == NULL) {
		p = createNode();
		p->records[0] = inRecord;
		p->count = 1;
		*root = p;

		res = createResult();
		SPLITS(res) = splits;
		return res;
	}

	found = 0;
	curr = *root;
	do {
		for(i = 0; i < curr->count; i++) {
			int cmp = strcmp(inRecord->name, curr->records[i]->name);
			if(cmp == 0) {
				found = 1;
				break;
			} else if(cmp < 0) {
				break;
			}
		}
		p = curr->pointers[i];
		if(p != NULL) {
			pushNode(curr);
			curr = p;
		}
	} while(!found && p != NULL);

	if(found) { // skip duplicate keys
		free(inRecord);
		clearNodes();
		return NULL;
	}

	p = NULL;
	finished = 0;
	do {
		if(curr->count < 2 * CAP_ORDER) {
			for(i = curr->count - 1; i >= 0; i--) { // search position from back
				if(strcmp(inRecord->name, curr->records[i]->name) > 0) {
					break;
				}
				curr->pointers[i + 2] = curr->pointers[i + 1];
				curr->records[i + 1] = curr->records[i];
			}
			curr->records[i + 1] = inRecord;
			curr->pointers[i + 2] = p;
			curr->count++;
			finished = 1;
		} else {
			type_bignode bnode;
			bnode.count = 0;

			/* Copy elements to bnode */
			bnode.pointers[0] = curr->pointers[0];
			for(i = 0; i < curr->count; i++) {
				// stop copying if inKey lies between current node's elements
				if(strcmp(curr->records[i]->name, inRecord->name) > 0) {
					break;
				}
				bnode.records[i] = curr->records[i];
				bnode.pointers[i + 1] = curr->pointers[i + 1];
				bnode.count++;
			}
			// Copy record-to-be-inserted in appropriate position
			bnode.records[i] = inRecord;
			bnode.pointers[i + 1] = p;
			bnode.count++;
			// Copy the remaining records in current node
			for(; i < curr->count; i++) {
				bnode.records[i + 1] = curr->records[i];
				bnode.pointers[i + 2] = curr->pointers[i + 1];
				bnode.count++;
			}
			//bnode.pointers[i + 2] = curr->pointers[i + 1];
			/* End copy */

			/* Copy the first half of bnode's records to the current node */
			curr->count = 0;
			curr->pointers[0] = bnode.pointers[0];
			for(i = 0; i < bnode.count / 2; i++) {
				curr->records[i] = bnode.records[i];
				curr->pointers[i + 1] = bnode.pointers[i + 1];
				curr->count++;
			}

			/* Create a new node and copy the remaining half, excluding one in the exact middle */
			newNode = createNode();
			newNode->pointers[0] = bnode.pointers[bnode.count / 2 + 1];
			for(i = 0; i < bnode.count / 2; i++) {
				newNode->records[i] = bnode.records[i + bnode.count / 2 + 1];
				newNode->pointers[i + 1] = bnode.pointers[i + bnode.count / 2 + 2];
				newNode->count++;
			}

			inRecord = bnode.records[bnode.count / 2];
			//strcpy(inKey, bnode.records[bnode.count / 2]->name);
			p = newNode;
			splits++;

			curr = popNode();
			if(curr == NULL) { // stack empty, curr was the root node
				newNode = createNode(); // new root node
				newNode->pointers[0] = *root;
				newNode->records[0] = inRecord;
				newNode->pointers[1] = p;
				newNode->count = 1;

				*root = newNode;
				finished = 1;
			}
		}
	} while(!finished);
	clearNodes();

	res = createResult();
	SPLITS(res) = splits;
	return res;
}

void redistribute_twobignode(type_node* left, type_rec** middle, type_node* right) {
	int i;
	type_twobignode bnode;
	bnode.count = 0;

	bnode.pointers[0] = left->pointers[0];
	for(i = 0; i < left->count; i++) {
		bnode.records[bnode.count] = left->records[i];
		bnode.pointers[bnode.count + 1] = left->pointers[i + 1];
		bnode.count++;
	}

	bnode.records[bnode.count] = *middle;
	bnode.count++;

	bnode.pointers[bnode.count] = right->pointers[0];
	for(i = 0; i < right->count; i++) {
		bnode.records[bnode.count] = right->records[i];
		bnode.pointers[bnode.count + 1] = left->pointers[i + 1];
		bnode.count++;
	}


	left->count = 0;
	left->pointers[0] = bnode.pointers[0];
	for(i = 0; i < bnode.count / 2; i++) {
		left->records[i] = bnode.records[i];
		left->pointers[i + 1] = bnode.pointers[i + 1];
		left->count++;
	}

	*middle = bnode.records[bnode.count / 2];

	right->count = 0;
	right->pointers[0] = bnode.pointers[bnode.count / 2 + 1];
	for(i = 0; i < bnode.count / 2; i++) {
		right->records[i] = bnode.records[i + bnode.count / 2 + 1];
		right->pointers[i + 1] = bnode.pointers[i + bnode.count / 2 + 2];
		right->count++;
	}
}

type_result* b_tree_deletation(char searchName[50], type_node** root) {
	type_node *curr, *p, *left, *right;
	type_result* res;
	int outPos, found, i, finished, position, redist, merging;

	outPos = 0;
	found = 0;
	curr = *root;
	p = NULL;
	do {
		for(i = 0; i < curr->count; i++) {
			int cmp = strcmp(searchName, curr->records[i]->name);
			if(cmp == 0) {
				outPos = i;
				found = 1;
				break;
			} else if(cmp < 0) {
				p = curr->pointers[i];
				if(p) {
					pushNode(curr);
					curr = p;
				}
			}
		}
		if(!found && strcmp(searchName, curr->records[curr->count - 1]->name) > 0) {
			p = curr->pointers[curr->count];
			if(p) {
				pushNode(curr);
				curr = p;
			}
		}
	} while(!found && p != NULL);

	if(!found) {
		clearNodes();
		return NULL;
	}

	if(curr->pointers[0]) {
		p = curr->pointers[outPos + 1];
		pushNode(curr);
		while(p->pointers[0]) {
			pushNode(p);
			p = p->pointers[0];
		}
		
		curr->records[outPos] = p->records[0];
		
		curr = p;
		outPos = 0;
	}

	free(curr->records[outPos]);
	redist = 0;
	merging = 0;

	finished = 0;
	do {
		for(i = outPos; i < curr->count - 1; i++) {
			curr->records[i] = curr->records[i + 1];
			curr->pointers[i + 1] = curr->pointers[i + 2];
		}
		curr->count--;

		if(curr == *root) {
			if(curr->count == 0) {
				*root = curr/* = *root */->pointers[0];
				free(curr);
			}
			finished = 1;
			break;
		}

		if(curr->count >= CAP_ORDER) {
			finished = 1;
			break;
		}

		p = popNode();
		
		left = NULL;
		right = NULL;
		position = -1;
		for(i = 0; i <= p->count; i++) {
			if(p->pointers[i] == curr) {
				position = i;
				if(i > 0) left = p->pointers[i - 1];
				if(i < p->count) right = p->pointers[i + 1];
				break;
			}
		}

		if(right && right->count > CAP_ORDER) {
			redistribute_twobignode(curr, &(p->records[position]), right);
			redist++;
			finished = 1;
			break;
		} else if(left && left->count > CAP_ORDER) {
			redistribute_twobignode(left, &(p->records[position - 1]), curr);
			redist++;
			finished = 1;
			break;
		} else {
			if(right) {
				left = curr;
			} else {
				right = curr;
				position--;
			}
			left->records[left->count] = p->records[position];
			left->pointers[left->count + 1] = right->pointers[0];
			left->count++;

			for(i = 0; i < right->count; i++) {
				left->records[left->count] = right->records[i];
				left->pointers[left->count + 1] = right->pointers[i + 1];
				left->count++;
			}
			free(right);

			curr = p;
			outPos = position;
			merging++;
		}
	} while(!finished);
	clearNodes();


	res = createResult();
	REDISTRIBUTIONS(res) = redist;
	MERGES(res) = merging;
	return res;
}

type_result* search(char sname[50], int level, type_node* root) {
	type_node *curr, *p, *parent;
	type_result* res;
	int i, j, found = 0;

	curr = root;
	for(i = 0; i < curr->count; i++) {
		int cmp = strcmp(sname, curr->records[i]->name);
		if(cmp < 0) {
			break;
		} else if(cmp == 0) {
			found = 1;
			break;
		}
	}

	if(!found) {
		p = curr->pointers[i];
		if(p) { // has child, find recursively
			pushNode(curr);
			curr = p;
			return search(sname, level + 1, curr);
		} else { // not found
			clearNodes();
			return NULL;
		}
	}

	res = createResult();
	LEVEL(res) = level;
	POSITION(res) = i;

	parent = popNode();
	if(parent != NULL) {
		for(j = 0; j <= parent->count; j++) {
			if(parent->pointers[j] == curr) break;
		}

		if(j == parent->count) {
			res->left = parent->records[j - 1];
		} else {
			res->right = parent->records[j];
		}
	} // else left, right = NULL
	clearNodes();
	return res;
}

type_result* b_tree_search(char companyName[50], type_node* root) {
	return search(companyName, 0, root);
}

void b_tree_sequential_print(FILE* output, type_node* node) {
	int i;
	if(node == NULL) return;

	// print recursively
	b_tree_sequential_print(output, node->pointers[0]);
	for(i = 0; i < node->count; i++) {
		fprintf(output, "%-30s %d\n", node->records[i]->name, node->records[i]->leng);
		b_tree_sequential_print(output, node->pointers[i + 1]);
	}
}

void b_tree_free(type_node* node) {
	int i;
	if(node == NULL) return;

	b_tree_free(node->pointers[0]);
	for(i = 0; i < node->count; i++) {
		free(node->records[i]);

		b_tree_free(node->pointers[i + 1]);
	}
	free(node);
}
