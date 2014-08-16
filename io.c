#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <bsd/sys/queue.h>

#include "io.h"
#include "choice.h"

void
chomp(char *str, ssize_t len)
{
	if (str[len - 1] == '\n')
		str[len - 1] = '\0';
}

struct choices *
read_choices()
{
	struct choices *cs;
	struct choice *c;
	char *line;
	size_t n;
	ssize_t len;

	if ((cs = malloc(sizeof(struct choices))) == NULL)
		err(1, "malloc");

	SLIST_INIT(cs);

	for (;;) {
		line = NULL;
		n = 0;
		if ((len = getline(&line, &n, stdin)) == -1)
			break;
		chomp(line, len);
		c = choice_new(line, 1);
		SLIST_INSERT_HEAD(cs, c, choices);
		free(line);
	}

	free(line);
	return cs;
}
