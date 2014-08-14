#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "trie.h"
#include "str_list.h"

int
main(int argc,char **argv)
{
	struct trie *t;
	struct str_list *l;
	
	t = trie_new();

	trie_insert(t, "A");
	trie_insert(t, "AB");
	trie_insert(t, "ABC");
	trie_insert(t, "ACB");
	trie_insert(t, "BCA");

	trie_print(t);

	printf("---------------------------\n");

	l = trie_all_strs(t);

	str_list_print(l); 

	str_list_free(l);
	trie_free(t);

	return 0;
}
