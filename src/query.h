#ifndef QUERY_H
#define QUERY_H

#include <stddef.h>

enum query_type_enum {
	q_standard,
	q_levenshtein
};

struct query;

struct query *query_create(const char *initial);
void query_destroy(struct query *);

/* query setters
 * the ones that can fail, return a non-zero error code
 * on failure ( just malloc stuff... )
 */
int query_set_text(struct query*, const char *text);
void query_set_type(struct query*, enum query_type_enum type);
void query_toggle_type(struct query*);
void query_delete_between(struct query*, size_t start, size_t end);
int query_insert(struct query*, size_t index, char);


/* query getters ( taking pointer to `const struct query` ) */
char query_at(const struct query*, size_t index);
const char *query_text(const struct query*);
size_t query_length(const struct query*);
enum query_type_enum query_type(const struct query*);

#endif /* QUERY_H */
