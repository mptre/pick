#ifndef LIST_H
#define LIST_H

#include "trie.h"

struct list {
	struct trie *value;
	struct list *next;
};

struct list	*list_new();
void		 list_free(struct list *);
void		 list_print(struct list *, char *, int);
void		 list_insert(struct list *, char *);

#endif /* LIST_H */
