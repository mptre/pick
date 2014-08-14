#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "trie.h"

struct list *
list_new()
{
	struct list *l;

	if ((l = malloc(sizeof(struct list))) == NULL)
		err(1, "malloc");

	l->value = NULL;
	l->next = NULL;

	return l;
}

void
list_free(struct list *l)
{
	if (l == NULL)
		return;

	trie_free(l->value);
	list_free(l->next);
	free(l);
}

void
list_print(struct list *l, char *buf, int maxlen)
{
	char *brbuf;

	if (l == NULL)
		return;

	/*
	 * When branching into the next child of the children trie list, the
	 * branch has to have it's own copy of the buffer to avoid
	 * interference between branches.
	 */
	if ((brbuf = calloc(maxlen, sizeof(brbuf))) == NULL)
		err(1, "calloc");
	strcpy(brbuf, buf);
	brbuf = trie_print(l->value, brbuf, maxlen);
	free(brbuf);

	list_print(l->next, buf, maxlen);
}

void
list_insert(struct list *children, char *str)
{
	if (children->value == NULL) {
		children->value = trie_new();
		children->value->ch = str[0];
	}

	if (children->value->ch == str[0]) {
		if (str[0] != '\0')
			trie_insert(children->value, str + 1);
		return;
	}

	if (children->next == NULL)
		children->next = list_new();

	list_insert(children->next, str);
}
