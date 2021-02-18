// Compile with a -I to a directory containing rbtree.h and include an rbtree.o
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "rbtree.h"

typedef struct node {
	int val;
	struct rb_node node;
} node_t;

static struct rb_root root = RB_ROOT;

static node_t *
rbn_search(struct rb_root *root, int val)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		node_t *data = container_of(node, node_t, node);
		int result;

		result = val - data->val;

		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return data;
	}
	return NULL;
}

static bool
rbn_insert(struct rb_root *root, node_t *data)
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	/* Figure out where to put new node */
	while (*new) {
		node_t *this = container_of(*new, node_t, node);
		int result = data->val - this->val;

		parent = *new;
		if (result < 0)
			new = &((*new)->rb_left);
		else if (result > 0)
			new = &((*new)->rb_right);
		else
			return false;
	}

	/* Add new node and rebalance tree. */
	rb_link_node(&data->node, parent, new);
	rb_insert_color(&data->node, root);

	return true;
}

int main(int argc, char **argv)
{
	int i;
	int start = 1;
	int end = 10;
	int erase = 0;
	node_t *node;
	struct rb_node *rbn;
	struct rb_node *next;

	if (argc >= 2) start = atoi(argv[1]);
	if (argc >= 3) end = atoi(argv[2]);
	if (argc >= 4) erase = atoi(argv[3]);

	for (i = start; i <= end; i++) {
		node = malloc(sizeof(node_t));
		node->val = i;

		rbn_insert(&root, node);
	}

	printf("Root: %p\n", root.rb_node);
	for (rbn = rb_first(&root); rbn; rbn = rb_next(rbn)) {
		node = rb_entry(rbn, node_t, node);
		printf("%p: %c ^ %p <- %p -> %p %d\n", rbn, rbn->__rb_parent_color & 1 ? 'R' : 'B', rbn->__rb_parent_color & ~3, rbn->rb_left, rbn->rb_right, node->val);
//		printf("Val %d\n", rb_entry(rbn, node_t, node)->val);
	}

	printf("\nRoot (postorder): %p\n", rb_entry(root.rb_node, node_t, node));
	for (rbn = rb_first_postorder(&root); rbn; rbn = rb_next_postorder(rbn)) {
		node = rb_entry(rbn, node_t, node);
		printf("%p: %c ^ %p <- %p -> %p %d\n", rbn, rbn->__rb_parent_color & 1 ? 'R' : 'B', rbn->__rb_parent_color & ~3, rbn->rb_left, rbn->rb_right, node->val);
	}

	if (!erase)
		erase = start + ((long)root.rb_node >> 12) % (end - start + 1);
	printf("\nErasing %d\n", erase);
	for (rbn = rb_first(&root); rbn; rbn = next) {
		next = rb_next(rbn);
		node = rb_entry(rbn, node_t, node);
		if (node->val == erase) {
			rb_erase(rbn, &root);
			continue;
		}
		printf("Val %d\n", rb_entry(rbn, node_t, node)->val);
	}

	printf("\nErased %d\n", erase);
	for (rbn = rb_first(&root); rbn; rbn = rb_next(rbn)) {
		node = rb_entry(rbn, node_t, node);
		printf("Val %d\n", rb_entry(rbn, node_t, node)->val);
	}
}
