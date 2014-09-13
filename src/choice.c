#include <ctype.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "choice.h"

struct choice *
choice_new(char *str, char *desc, float score)
{
	struct choice *c;
	
	if ((c = malloc(sizeof(struct choice))) == NULL)
		err(1, "malloc");
	c->str = strdup(str);
	c->desc = strdup(desc);
	c->score = score;
	return c;
}

void
choice_free(struct choice *c)
{
	free(c->str);
	free(c->desc);
	free(c);
}
