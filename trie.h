#ifndef TRIE_H
#define TRIE_H

#include "trie_list.h"

struct trie {
	char ch;
	struct trie_list *children;
};

struct trie	*trie_new();
void		 trie_free(struct trie *);
void		 trie_print(struct trie *);
void		 trie_insert(struct trie *, char *);

#endif /* TRIE_H */
