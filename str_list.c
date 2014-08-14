#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "str_list.h"

struct str_list*
str_list_new()
{
	struct str_list *l;

	if ((l = malloc(sizeof(struct str_list))) == NULL)
		err(1, "malloc");

	l->str = NULL;
	l->next = NULL;

	return l;
}

void
str_list_print(struct str_list *l)
{
	if (l == NULL || l->str == NULL)
		return;

	printf("%s\n", l->str);
	str_list_print(l->next);
}

void
str_list_free(struct str_list *l)
{
	if (l == NULL)
		return;

	str_list_free(l->next);
	free(l->str);
	free(l);
}
