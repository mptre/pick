#ifndef CHOICE_H
#define CHOICE_H

#include <bsd/sys/queue.h>

struct choice {
    char *str;
    float score;
    SLIST_ENTRY(choice) choices;          /* List. */
};

SLIST_HEAD(choices, choice);

struct choice   *choice_new(char *, float);
void             choices_score(struct choices *, char *);
void             choices_sort(struct choices *);

#endif /* CHOICHE_H */
