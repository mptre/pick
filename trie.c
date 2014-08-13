#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "trie.h"

struct trie *
trie_new()
{
	struct trie *root;

	if ((root = malloc(sizeof(struct trie))) == NULL)
		err(1, "malloc");

	root->ch = 'r';
	root->children = list_new();

	return root;
}

void
trie_free(struct trie *t)
{
	if (t == NULL)
		return;

	list_free(t->children);
	free(t);
}

void
trie_print(struct trie *t, char *str, int maxlen)
{
	int len;

	if (t == NULL)
		return;

	if ((len = strlen(str)) == maxlen - 1) {
                maxlen = 2 * maxlen;
                if ((realloc(str, maxlen)) == NULL)
			err(1, "realloc");
	}

	str[len] = t->ch;

	if (str[len] == '\0')
		printf("%s\n", str);
	else 
		list_print(t->children, str, maxlen);
}

void
trie_insert(struct trie *t, char *str)
{
	if (str[0] == '\0')
		return;

	if (t->children == NULL)
		t->children = list_new();

	list_insert(t->children, str);
}
