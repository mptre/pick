#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bsd/sys/queue.h>

#include "choice.h"
#include "io.h"
#include "ui.h"

int
main(int argc,char **argv)
{

	struct choices *cs;
	char *sel;
	struct choice *np;

	cs = read_choices();

	sel = run_ui(cs);
	printf("%s\n", sel);

	while (!SLIST_EMPTY(cs)) {
		np = SLIST_FIRST(cs);
		SLIST_REMOVE_HEAD(cs, choices);
		free(np->str);
		free(np);
	}

	free(cs);

	return 0;
}
