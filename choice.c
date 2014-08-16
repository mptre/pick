#include <bsd/sys/queue.h>
#include <ctype.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "choice.h"

struct choice *
choice_new(char *str, float score)
{
	struct choice *c;
	
	if ((c = malloc(sizeof(struct choice))) == NULL)
		err(1, "malloc");

	c->str = strdup(str);
	c->score = score;
	
	return c;
}

size_t
match_len(char *str, char *query)
{
	size_t len;
	size_t i;
	size_t j;
	size_t x;

	for (len = 0, i = 0; str[i] != '\0'; ++i)
		if (tolower(str[i]) == tolower(query[0])) {
			for (j = 1, x = i + 1; query[j] != '\0'; ++j)
				for (;; ++x) {
					if (str[x] == '\0')
						return len;
					if (tolower(str[x]) == tolower(query[j]))
						break;
				}
			if (len == 0 || len > x - i + 1)
				len = x - i + 1;
		}

	return len;
}

float
score_str(char *str, char *query)
{
	size_t slen;
	size_t qlen;
	size_t mlen;

	slen = strlen(str);
	qlen = strlen(query);

	if (qlen == 0)
		return 1;	

	if (slen == 0)
		return 0;

	if ((mlen = match_len(str, query)) == 0)
		return 0;

	return (float)qlen / (float)mlen / (float)slen;
}


void
choices_score(struct choices *cs, char *query)
{
	struct choice *c;

	SLIST_FOREACH(c, cs, choices) {
		c->score = score_str(c->str, query);
	}
}

struct choice *
merge(struct choice *front, struct choice *back)
{
	struct choice head;
	struct choice *c;
	c = &head;

	while (front != NULL && back != NULL) {
		if (front->score > back->score) {
			c->choices.sle_next = front;
			c = front;
			front = front->choices.sle_next;
		} else {
			c->choices.sle_next = back;
			c = back;
			back = back->choices.sle_next;
		}
	}
	if (front != NULL)
		c->choices.sle_next = front;
	else
		c->choices.sle_next = back;

	return head.choices.sle_next;
}

struct choice *
merge_sort(struct choice *c)
{
	struct choice *front;
	struct choice *back;

	if (c == NULL || c->choices.sle_next == NULL)
		return c;

	front = c;
	back = c->choices.sle_next;

	while (back != NULL && back->choices.sle_next != NULL) {
		c = c->choices.sle_next;
		back = back->choices.sle_next->choices.sle_next;
	}
	back = c->choices.sle_next;
	c->choices.sle_next = NULL;

	return merge(merge_sort(front), merge_sort(back));
}

void
choices_sort(struct choices *cs)
{
	cs->slh_first = merge_sort(cs->slh_first);
}
