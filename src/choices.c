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

static size_t min_match_length(char *, char *);
static float score(char *, char *);
static struct choice *merge(struct choice *, struct choice *);
static struct choice *sort(struct choice *);

void
choices_score(struct choices *choices, char *query)
{
	struct choice *choice;

	SLIST_FOREACH(choice, choices, choices) {
		choice->score = score(choice->string, query);
	}
}

void
choices_sort(struct choices *choices)
{
	choices->slh_first = sort(choices->slh_first);
}

void
choices_free(struct choices *choices)
{
	struct choice *choice;

	while (!SLIST_EMPTY(choices)) {
		choice = SLIST_FIRST(choices);
		SLIST_REMOVE_HEAD(choices, choices);
		choice_free(choice);
	}

	free(choices);
}

size_t
min_match_length(char *string, char *query)
{
	size_t match_length, match_start, query_position, match_position;
	int query_char, query_start;

	query_start = tolower((unsigned char)query[0]);

	for (match_length = 0, match_start = 0; string[match_start] != '\0';
	    ++match_start) {
		if (tolower((unsigned char)string[match_start]) ==
		    query_start) {
			for (query_position = 1,
			    match_position = match_start + 1;
			    query[query_position] != '\0'; ++query_position) {
				query_char = tolower(
				    (unsigned char)query[query_position]);

				for (;; ++match_position) {
					if (string[match_position] == '\0') {
						return match_length;
					}

					if (tolower((unsigned char)string[match_position]) == query_char) {
						++match_position;
						break;
					}
				}
			}
			if (match_length == 0 || match_length > match_position -
			    match_start + 1) {
				match_length = match_position - match_start + 1;
			}
		}
	}

	return match_length;
}

static float
score(char *string, char *query)
{
	size_t string_length, query_length, match_length;

	string_length = strlen(string);
	query_length = strlen(query);

	if (query_length == 0) {
		return 1;	
	}

	if (string_length == 0) {
		return 0;
	}

	match_length = min_match_length(string, query);
	if (match_length == 0) {
		return 0;
	}

	return (float)query_length / (float)match_length / (float)string_length;
}

static struct choice *
merge(struct choice *front, struct choice *back)
{
	struct choice head;
	struct choice *choice;

	choice = &head;

	while (front != NULL && back != NULL) {
		if (front->score > back->score ||
		    (front->score == back->score &&
		     strcmp(front->string, back->string) < 0)) {
			choice->choices.sle_next = front;
			choice = front;
			front = front->choices.sle_next;
		} else {
			choice->choices.sle_next = back;
			choice = back;
			back = back->choices.sle_next;
		}
	}

	if (front != NULL) {
		choice->choices.sle_next = front;
	} else {
		choice->choices.sle_next = back;
	}

	return head.choices.sle_next;
}

static struct choice *
sort(struct choice *choice)
{
	struct choice *front, *back;

	if (choice == NULL || choice->choices.sle_next == NULL) {
		return choice;
	}

	front = choice;
	back = choice->choices.sle_next;

	while (back != NULL && back->choices.sle_next != NULL) {
		choice = choice->choices.sle_next;
		back = back->choices.sle_next->choices.sle_next;
	}

	back = choice->choices.sle_next;
	choice->choices.sle_next = NULL;

	return merge(sort(front), sort(back));
}
