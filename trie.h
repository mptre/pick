#ifndef TRIE_H
#define TRIE_H

#include "list.h"

struct trie {
	char ch;
	struct list *children;
};

struct trie	*trie_new();
void		 trie_free(struct trie *);
void		 trie_print(struct trie *, char *, int);
void		 trie_insert(struct trie *, char *);

#endif /* TRIE_H */
