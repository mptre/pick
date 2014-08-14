#include <stdio.h>
#include <string.h>

#include "io.h"
#include "trie.h"

void
chomp(char *str)
{
	int last;
	last = strlen(str) - 1;
	if (str[last] == '\n')
		str[last] = '\0';
}

struct trie *
read_choices()
{
	struct trie *t;
	char string[80];

	t = trie_new();
	while (fgets(string, 80, stdin) != NULL) {
		chomp(string);
		trie_insert(t, string);
	}

	return t;
}
