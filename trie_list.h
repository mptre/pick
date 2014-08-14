#ifndef TRIE_LIST_H
#define TRIE_LIST_H

#include "trie.h"

struct trie_list {
	struct trie *trie;
	struct trie_list *next;
};

struct trie_list	*trie_list_new();
void        		 trie_list_free(struct trie_list *);
void        		 trie_list_print(struct trie_list *, char *, int);
void	        	 trie_list_insert(struct trie_list *, char *);
struct str_list		*trie_list_all_strs(struct trie_list *, char *, int,
		struct str_list *);

#endif /* TRIE_LIST_H */
