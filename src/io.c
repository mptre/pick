#include <err.h>
#define _WITH_GETLINE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_FULL_QUEUE_H
#include <sys/queue.h>
#else /* HAVE_FULL_QUEUE_H */
#include "compat/queue.h"
#endif /* HAVE_FULL_QUEUE_H */

#include "io.h"
#include "choice.h"
#include "choices.h"

static void chomp(char *, ssize_t);

struct choices *
get_choices(int parse_desc)
{
	struct choices *cs;
	struct choice *c;
	char *line;
	char *desc;
	char *ifs;
	size_t n;
	ssize_t len;

	if ((ifs = getenv("IFS")) == NULL)
		ifs = " ";

	if ((cs = malloc(sizeof(struct choices))) == NULL)
		err(1, "malloc");

	SLIST_INIT(cs);

	for (;;) {
		line = NULL;
		desc = "";
		n = 0;
		if ((len = getline(&line, &n, stdin)) == -1)
			break;
		chomp(line, len);
		if (parse_desc)
			strtok_r(line, ifs, &desc);
		c = choice_new(line, desc, 1);
		SLIST_INSERT_HEAD(cs, c, choices);
		free(line);
	}

	free(line);
	return cs;
}

void
put_choice(struct choice *c, int output_desc)
{
	printf("%s\n", c->str);
	if (output_desc)
		printf("%s\n", c->desc);
}

static void
chomp(char *str, ssize_t len)
{
	if (str[len - 1] == '\n')
		str[len - 1] = '\0';
}
