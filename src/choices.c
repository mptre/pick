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
min_match_length(char *candidate, char *query, size_t *match_start_po)
{
	size_t min_match_len;
	size_t match_start_pos;
	size_t query_pos;
	size_t match_pos;

  int matched = 0;

  // loop over query candidateing
	for (min_match_len = 0, match_start_pos = 0; candidate[match_start_pos] != '\0'; ++match_start_pos) {
    // check if there's a match with the first query character
		if (tolower(candidate[match_start_pos]) == tolower(query[0])) {
      if (matched == 0) {
        matched = 1;
        *match_start_po = match_start_pos;
      }
      // loop over remaining characters in the query candidateing
			for (query_pos = 1, match_pos = match_start_pos + 1; query[query_pos] != '\0'; ++query_pos) {

        // see if there's a matching character in
        // the remaining characters of the filename
				for (;; ++match_pos) {
					if (candidate[match_pos] == '\0') {
						return min_match_len;
          }

					if (tolower(candidate[match_pos]) == tolower(query[query_pos])) {
						++match_pos;
						break; // move on to next query character
					}
				}
      }

      size_t curr_match_len = match_pos - match_start_pos + 1;
			if (min_match_len == 0 || min_match_len > curr_match_len)
				min_match_len = curr_match_len;
		}
  }

	return min_match_len;
}

float
score_str(char *str, char *query, size_t *start_pos, size_t *match_len)
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
	if ((mlen = min_match_length(str, query, start_pos)) == 0)
		return 0;
  *match_len = mlen;
	return (float)qlen / (float)mlen / (float)slen;
}


void
choices_score(struct choices *cs, char *query)
{
	struct choice *c;
  size_t start_pos = 0;
  size_t match_len = 0;

	SLIST_FOREACH(c, cs, choices) {
		c->score = score_str(c->str, query, &start_pos, &match_len);
    c->start_pos = start_pos;
    c->match_len = match_len;
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
