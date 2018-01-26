// Compile from parent directory with: gcc -I lib -o test/list test/list.c lib/liblib.a

#include <stdio.h>
#include <stdbool.h>

#include "list.h"

int main(int argc, char **argv)
{
	list l = NULL;
	element e, n;
	int i;
	char *str;
	bool del;

	printf("\nNo list\n\n");
	LIST_FOREACH_NEXT(l, str, e, n) {
		printf("e: %p, n: %p, %s\n", e, n, str);
	}

	l = alloc_list(NULL, NULL);

	printf("\nEmpty list\n\n");
	LIST_FOREACH_NEXT(l, str, e, n) {
		printf("e: %p, n: %p, %s\n", e, n, str);
	}

	for (i = 1; i <argc; i++)
		list_add(l, argv[i]);

	printf("\nForeach\n\n");
	LIST_FOREACH(l, str, e) {
		printf("e: %p %s\n", e, str);
	}

	printf("\nForeach_next - del odd entries\n\n");
	del = true;
	LIST_FOREACH_NEXT(l, str, e, n) {
		printf("e: %p, n: %p, %s\n", e, n, str);
		if (del)
			free_list_element(l, e);
		del = !del;
	}

	printf("\nForeach_next - del all\n\n");
	LIST_FOREACH_NEXT(l, str, e, n) {
		printf("e: %p, n: %p, %s\n", e, n, str);
		free_list_element(l, e);
	}

	printf("\nForeach - emptied list\n\n");
	LIST_FOREACH(l, str, e) {
		printf("e: %p %s\n", e, str);
	}

}
