#include <stdlib.h>
#include <err.h>

#include "trie.h"

int
main(int argc,char **argv)
{
	struct trie *t;
	
	t = trie_new();

	trie_insert(t, "A");
	trie_insert(t, "AB");
	trie_insert(t, "ABC");
	trie_insert(t, "ACB");
	trie_insert(t, "BCA");

	trie_print(t);
	trie_free(t);

	return 0;
}
