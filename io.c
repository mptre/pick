#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "trie.h"

void
chomp(char *str, ssize_t len)
{
	if (str[len - 1] == '\n')
		str[len - 1] = '\0';
}

struct trie *
read_choices()
{
	struct trie *t;
	char *line;
	size_t n;
	ssize_t len;

	t = trie_new();
	for (;;) {
		line = NULL;
		n = 0;
		if ((len = getline(&line, &n, stdin)) == -1)
			break;
		chomp(line, len);
		trie_insert(t, line);
		free(line);
	}

	free(line);
	return t;
}
