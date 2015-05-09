#ifndef CHOICES_H
#define CHOICES_H

#ifdef HAVE_FULL_QUEUE_H
#include <sys/queue.h>
#else /* HAVE_FULL_QUEUE_H */
#include "compat/queue.h"
#endif /* HAVE_FULL_QUEUE_H */

#include "choice.h"

SLIST_HEAD(choices, choice);

void	choices_score(struct choices *, char *);
void	choices_sort(struct choices *);
void	choices_free(struct choices *);
#endif /* CHOICES_H */
