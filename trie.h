#ifndef TRIE_H
#define TRIE_H

#include "str_list.h"
#include "trie_list.h"

struct trie {
	char ch;
	struct trie_list *children;
};

struct trie 	*trie_new();
void    		 trie_free(struct trie *);
void    		 trie_print(struct trie *);
void    		 trie_insert(struct trie *, char *);
struct str_list *trie_all_strs(struct trie *);

#endif /* TRIE_H */
