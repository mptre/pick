#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "trie_list.h"
#include "trie.h"

struct trie_list *
trie_list_new()
{
	struct trie_list *l;

	if ((l = malloc(sizeof(struct trie_list))) == NULL)
		err(1, "malloc");

	l->trie = NULL;
	l->next = NULL;

	return l;
}

void
trie_list_free(struct trie_list *l)
{
	if (l == NULL)
		return;

	trie_free(l->trie);
	trie_list_free(l->next);
	free(l);
}

char *
_trie_print(struct trie *t, char *str, int maxlen)
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
		trie_list_print(t->children, str, maxlen);

	return str;
}


void
trie_list_print(struct trie_list *l, char *buf, int maxlen)
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
	brbuf = _trie_print(l->trie, brbuf, maxlen);
	free(brbuf);

	trie_list_print(l->next, buf, maxlen);
}

void
trie_list_insert(struct trie_list *children, char *str)
{
	if (children->trie == NULL) {
		children->trie = trie_new();
		children->trie->ch = str[0];
	}

	if (children->trie->ch == str[0]) {
		if (str[0] != '\0')
			trie_insert(children->trie, str + 1);
		return;
	}

	if (children->next == NULL)
		children->next = trie_list_new();

	trie_list_insert(children->next, str);
}
