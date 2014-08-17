#ifndef CHOICE_H
#define CHOICE_H

#include <bsd/sys/queue.h>

struct choice {
    char *str;
    float score;
    SLIST_ENTRY(choice) choices;          /* List. */
};

struct choice   *choice_new(char *, float);
void             choice_free(struct choice *);

#endif /* CHOICHE_H */
