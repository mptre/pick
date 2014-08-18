#include <ctype.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef HAVE_FULL_QUEUE_H
#include <sys/queue.h>
#else /* HAVE_FULL_QUEUE_H */
#include "compat/queue.h"
#endif /* HAVE_FULL_QUEUE_H */

#include "choice.h"
#include "choices.h"

size_t
min_match_length(char *str, char *query)
{
	size_t mlen;
	size_t mstart;
	size_t qpos;
	size_t mpos;

	for (mlen = 0, mstart = 0; str[mstart] != '\0'; ++mstart)
		if (tolower(str[mstart]) == tolower(query[0])) {
			for (qpos = 1, mpos = mstart + 1; query[qpos] != '\0'; ++qpos)
				for (;; ++mpos) {
					if (str[mpos] == '\0')
						return mlen;
					if (tolower(str[mpos]) == tolower(query[qpos])) {
						++mpos;
						break;
					}
				}
			if (mlen == 0 || mlen > mpos - mstart + 1)
				mlen = mpos - mstart + 1;
		}
	return mlen;
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
	if ((mlen = min_match_length(str, query)) == 0)
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
sort(struct choice *c)
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

	return merge(sort(front), sort(back));
}

void
choices_sort(struct choices *cs)
{
	cs->slh_first = sort(cs->slh_first);
}

void
choices_free(struct choices *cs)
{
	struct choice *c;

	while (!SLIST_EMPTY(cs)) {
		c = SLIST_FIRST(cs);
		SLIST_REMOVE_HEAD(cs, choices);
		choice_free(c);
	}
	free(cs);
}
