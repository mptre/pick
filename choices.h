#ifndef CHOICES_H
#define CHOICES_H

#include <bsd/sys/queue.h>

#include "choice.h"

SLIST_HEAD(choices, choice);

void             choices_score(struct choices *, char *);
void             choices_sort(struct choices *);
void             choices_free(struct choices *);
#endif /* CHOICES_H */
