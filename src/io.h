#ifndef IO_H
#define IO_H

#include "choices.h"
#include "choice.h"

struct choices	*io_read_choices(int);
void		 io_print_choice(struct choice *, int);

#endif /* IO_H */
