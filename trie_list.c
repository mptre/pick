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
_trie_print(struct trie *t, char *buf, int bufsize)
{
	int len;

	if (t == NULL)
		return buf;

	if ((len = strlen(buf)) == bufsize - 1) {
                bufsize = 2 * bufsize;
                if ((buf = realloc(buf, bufsize * sizeof(buf))) == NULL)
			err(1, "realloc");
	}

	buf[len] = t->ch;

	if (buf[len] == '\0')
		printf("%s\n", buf);
	else 
		trie_list_print(t->children, buf, bufsize);

	return buf;
}


void
trie_list_print(struct trie_list *l, char *buf, int bufsize)
{
	char *brbuf;

	if (l == NULL)
		return;

	/*
	 * When branching into the next child of the children trie list, the
	 * branch has to have it's own copy of the buffer to avoid
	 * interference between branches.
	 */
	if ((brbuf = calloc(bufsize, sizeof(brbuf))) == NULL)
		err(1, "calloc");
	strcpy(brbuf, buf);
	brbuf = _trie_print(l->trie, brbuf, bufsize);
	free(brbuf);

	trie_list_print(l->next, buf, bufsize);
}

struct str_list *
_trie_all_strs(struct trie *t, char *buf, int bufsize, struct str_list *sl)
{
	int len;

	if (t == NULL)
		return sl;

	len = strlen(buf);

	buf[len] = t->ch;

	if (buf[len] == '\0') {
		sl->next = str_list_new();
		sl = sl->next;
		if ((sl->str = strdup(buf)) == NULL)
			err(1, "strdup");
	} else  {
		sl = trie_list_all_strs(t->children, buf, bufsize, sl);
	}

	return sl;
}

struct str_list *
trie_list_all_strs(struct trie_list *l, char *buf, int bufsize, struct str_list *sl)
{
	char *brbuf;
	int len;

	if (l == NULL)
		return sl;

	/*
	 * When branching into the next child of the children trie list, the
	 * branch has to have it's own copy of the buffer to avoid
	 * interference between branches.
	 */
	if ((len = strlen(buf)) == bufsize - 1)
                bufsize = 2 * bufsize;
	if ((brbuf = calloc(bufsize, sizeof(char))) == NULL)
		err(1, "calloc");
	strcpy(brbuf, buf);
	sl = _trie_all_strs(l->trie, brbuf, bufsize, sl);
	free(brbuf);

	return trie_list_all_strs(l->next, buf, bufsize, sl);
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
