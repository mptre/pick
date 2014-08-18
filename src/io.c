#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_FULL_QUEUE_H
#include <sys/queue.h>
#else /* HAVE_FULL_QUEUE_H */
#include "compat/queue.h"
#endif /* HAVE_FULL_QUEUE_H */

#include "io.h"
#include "choice.h"
#include "choices.h"

void
chomp(char *str, ssize_t len)
{
	if (str[len - 1] == '\n')
		str[len - 1] = '\0';
}

struct choices *
get_choices()
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

void
put_choice(struct choice *c)
{
	printf("%s\n", c->str);
}
