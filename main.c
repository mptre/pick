#include <stdlib.h>
#include <err.h>
#include "trie.h"

int
main(int argc,char **argv)
{
	struct trie *t;
	char *str;
	
	t = trie_new();

	trie_insert(t, "A");
	trie_insert(t, "B");
	/*
	trie_insert(t, "ActiveRecord::Base");
	trie_insert(t, "ActiveRecord::Persistance");
	*/

	if ((str = calloc(64, sizeof(str))) == NULL)
		err(1, "calloc");
	trie_print(t, str, 64);
	free(str);
	trie_free(t);

	return 0;
}
