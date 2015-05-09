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
io_read_choices(int read_descriptions)
{
	char *line, *description, *field_separator;
	size_t line_size;
	ssize_t length;
	struct choice *choice;
	struct choices *choices;

	field_separator = getenv("IFS");
	if (field_separator == NULL) {
		field_separator = " ";
	}

	choices = malloc(sizeof(struct choices));
	if (choices == NULL) {
		err(1, "malloc");
	}

	SLIST_INIT(choices);

	for (;;) {
		line = NULL;
		description = "";
		line_size = 0;

		length = getline(&line, &line_size, stdin);
		if (length == -1) {
			break;
		}

		chomp(line, length);

		if (read_descriptions) {
			strtok_r(line, field_separator, &description);
		}

		choice = choice_new(line, description, 1);
		SLIST_INSERT_HEAD(choices, choice, choices);

		free(line);
	}

	free(line);

	return choices;
}

void
io_print_choice(struct choice *choice, int output_description)
{
	printf("%s\n", choice->string);

	if (output_description) {
		printf("%s\n", choice->description);
	}
}

static void
chomp(char *string, ssize_t length)
{
	if (string[length - 1] == '\n') {
		string[length - 1] = '\0';
	}
}
