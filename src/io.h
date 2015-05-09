#ifndef IO_H
#define IO_H

#include "choices.h"
#include "choice.h"

struct choices	*get_choices(int);
void		 put_choice(struct choice *, int);

#endif /* IO_H */
