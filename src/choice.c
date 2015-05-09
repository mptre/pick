#include <ctype.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "choice.h"

struct choice *
choice_new(char *string, char *description, float score)
{
	struct choice *choice;

	choice = malloc(sizeof(struct choice));
	if (choice == NULL) {
		err(1, "malloc");
	}

	choice->string = strdup(string);
	choice->description = strdup(description);
	choice->score = score;

	return choice;
}

void
choice_free(struct choice *choice)
{
	free(choice->string);
	free(choice->description);
	free(choice);
}
