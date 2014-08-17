#include <stdlib.h>

#include "choices.h"
#include "io.h"
#include "ui.h"

int
main(int argc,char **argv)
{
	struct choices *cs;

	cs = get_choices();
	put_choice(get_selected(cs));
	choices_free(cs);
	return 0;
}
