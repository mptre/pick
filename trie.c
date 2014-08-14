#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str_list.h"
#include "trie_list.h"
#include "trie.h"

static const int bufsize = 64;

struct trie *
trie_new()
{
	struct trie *root;

	if ((root = malloc(sizeof(struct trie))) == NULL)
		err(1, "malloc");

	root->children = trie_list_new();

	return root;
}

void
trie_free(struct trie *t)
{
	if (t == NULL)
		return;

	trie_list_free(t->children);
	free(t);
}

void
trie_print(struct trie *t)
{
	char *buf;

	if ((buf = calloc(bufsize, sizeof(buf))) == NULL)
		err(1, "calloc");
	trie_list_print(t->children, buf, bufsize);
	free(buf);
}

void
trie_insert(struct trie *t, char *str)
{
	trie_list_insert(t->children, str);
}

struct str_list *
trie_all_strs(struct trie *t)
{
	char *buf;
	struct str_list *l;

	l = str_list_new();
	if ((buf = calloc(bufsize, sizeof(char))) == NULL)
		err(1, "calloc");
	trie_list_all_strs(t->children, buf, bufsize, l);
	free(buf);

	return l;
}
