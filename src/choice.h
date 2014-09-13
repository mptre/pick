#ifndef CHOICE_H
#define CHOICE_H

#ifdef HAVE_FULL_QUEUE_H
#include <sys/queue.h>
#else /* HAVE_FULL_QUEUE_H */
#include "compat/queue.h"
#endif /* HAVE_FULL_QUEUE_H */

struct choice {
    char *str;
    char *desc;
    float score;
    SLIST_ENTRY(choice) choices;          /* List. */
};

struct choice   *choice_new(char *, char *, float);
void             choice_free(struct choice *);

#endif /* CHOICHE_H */
