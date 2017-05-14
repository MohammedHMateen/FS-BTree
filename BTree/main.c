#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btree.h"

void readName(char* companyName) {
	char c;
	while((c = getchar()) == ' ' || c == '\r' || c == '\n');
	companyName[0] = c;
	gets(companyName + 1);
	companyName[strcspn(companyName, "\r\n")] = '\0';
}

int main() {
	FILE *file;
	char companyName[50], command[10];
	type_result* result;
	type_node* root = NULL;

	file = fopen("Com_names1.txt", "r");
	while(fgets(companyName, 50, file)) {
		companyName[strcspn(companyName, "\r\n")] = '\0'; // truncate newline character

		result = b_tree_insertation(companyName, &root);
		free(result);
	}
	fclose(file);

	file = fopen("Com_names2.txt", "r");
	while(fgets(companyName, 50, file)) {
		companyName[strcspn(companyName, "\r\n")] = '\0';

		result = b_tree_insertation(companyName, &root);
		free(result);
	}
	fclose(file);

	do {
		printf("Command? ");
		scanf("%s", command);
		if(strcmp(command, "r") == 0) {
			/* B-Tree search */
			readName(companyName);
			result = b_tree_search(companyName, root);
			if(result) {
				char* suffix;
				/* Find appropriate suffix for position number */
				switch(POSITION(result) % 10) {
				case 1:
					suffix = "st"; break;
				case 2:
					suffix = "nd"; break;
				case 3:
					suffix = "rd"; break;
				default:
					suffix = "th"; break;
				}
				printf("Found %s, Level %d, %d%s record", companyName, LEVEL(result), POSITION(result), suffix);

				if(LEFT_PARENT(result)) {
					printf(", Right child of %s.", LEFT_PARENT(result)->name);
				} else if(RIGHT_PARENT(result)) {
					printf(", Left child of %s.", RIGHT_PARENT(result)->name);
				} else {
					printf(". (Root node)");
				}
				putchar('\n');
				free(result);
			} else {
				printf("%s not found.\n", companyName);
			}

		} else if(strcmp(command, "d") == 0) {
			/* B-Tree deletation */
			readName(companyName);
			result = b_tree_deletation(companyName, &root);
			if(result) {
				printf("Deleted %s, %d redistributions, %d merges.\n", companyName, REDISTRIBUTIONS(result), MERGES(result));
				free(result);
			} else {
				printf("%s not found.\n", companyName);
			}

		} else if(strcmp(command, "sp") == 0) {
			/* Sequential print */
			file = fopen("SeqData.txt", "w");
			b_tree_sequential_print(file, root);
			fclose(file);

		} else if(strcmp(command, "i") == 0) {
			/* B-Tree Insertation */
			readName(companyName);
			result = b_tree_insertation(companyName, &root);
			if(result) {
				printf("Inserted %s, %d splits.\n", companyName, SPLITS(result));
				free(result);
			} else {
				printf("%s already exists.\n", companyName);
			}

		} else if(strcmp(command, "exit") == 0) {
			break;
		}
	} while(1);

	/* Free memory before exiting */
	b_tree_free(root);
	return 0;
}
