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

char *
trie_print(struct trie *t, char *str, int maxlen)
{
	int len;

	if (t == NULL)
		return str;

	if ((len = strlen(str)) == maxlen - 1) {
                maxlen = 2 * maxlen;
                if ((str = realloc(str, maxlen * sizeof(str))) == NULL)
			err(1, "realloc");
	}

	str[len] = t->ch;

	if (str[len] == '\0')
		printf("%s\n", str);
	else 
		list_print(t->children, str, maxlen);

	return str;
}

void
trie_insert(struct trie *t, char *str)
{
	list_insert(t->children, str);
}
