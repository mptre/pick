
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "query.h"

struct query {
	char *text;
	size_t length;
	enum query_type_enum type;
};

struct query *query_create(const char *initial)
{
	struct query *query;

	assert(initial != NULL);

	query = malloc(sizeof(*query));
	if (query == NULL) {
		return NULL;
	}
	query->text = NULL;
	if (query_set_text(query, initial) != 0) {
		free(query);
		return NULL;
	}
	query->type = q_standard;
	return query;
}

void query_destroy(struct query *query)
{
	if (query != NULL) {
		free(query->text);
		free(query);
	}
}

int query_set_text(struct query *query, const char *text)
{
	size_t new_length;
	char *text_cpy;

	assert(query != NULL);

	new_length = strlen(text);
	text_cpy = realloc(query->text, new_length + 1);
	if (text_cpy == NULL) {
		return -1;
	}
	strcpy(text_cpy, text);
	query->length = new_length;
	query->text = text_cpy;
	return 0;
}

void query_set_type(struct query *query, enum query_type_enum type)
{
	assert(query != NULL);
	assert(type == q_standard || type == q_levenshtein);

	query->type = type;
}

void query_toggle_type(struct query *query)
{
	assert(query != NULL);

	query->type = (query->type == q_standard) ? q_levenshtein : q_standard;
}

void query_delete_between(struct query *query, size_t start, size_t end)
{
	size_t t;

	assert(query != NULL);

	if (query->length == 0) {
		return;
	}
	if (start > query->length) {
		start = query->length;
	}
	if (end > query->length) {
		end = query->length;
	}
	if (start == end) {
		return;
	}
	if (start > end) {
		t = end;
		end = start;
		start = t;
	}
	memmove(query->text + start, query->text + end, query->length - end + 1);
	query->length -= end - start;
}

int query_insert(struct query *query, size_t index, char ch)
{
	char *new_p;

	assert(query != NULL);

	if (!isprint((unsigned char)ch)) {
		return -2;
	}
	new_p = realloc(query->text, query->length + 2);
	if (new_p == NULL) {
		return -1;
	}
	query->text = new_p;
	query->length++;
	if (index > query->length) {
		index = query->length;
	}
	memmove(query->text + index + 1,
		query->text + index, query->length - index);
	query->text[index] = ch;
	return 0;
}

char query_at(const struct query *query, size_t index)
{
	assert(query != NULL);

	if (index < query->length) {
		return query->text[index];
	}
	else {
		return '\0';
	}
}

const char *query_text(const struct query *query)
{
	assert(query != NULL);

	return query->text;
}

size_t query_length(const struct query *query)
{
	assert(query != NULL);

	return query->length;
}

enum query_type_enum query_type(const struct query *query)
{
	assert(query != NULL);

	return query->type;
}

