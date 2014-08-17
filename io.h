#ifndef IO_H
#define IO_H

#include "choices.h"
#include "choice.h"

struct choices  *get_choices();
void             put_choice(struct choice *);

#endif /* IO_H */
