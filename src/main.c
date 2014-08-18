#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#include "choices.h"
#include "io.h"
#include "ui.h"

void usage();

int
main(int argc,char **argv)
{
	int ch;
	struct choices *cs;

	while ((ch = getopt(argc, argv, "h")) != -1)
		switch (ch) {
		default:
			usage();
		}
	argc -= optind;
	argv += optind;


	cs = get_choices();
	put_choice(get_selected(cs));
	choices_free(cs);
	return EX_OK;
}

void
usage()
{
	fprintf(stderr, "usage: pick [-h]\n");
	fprintf(stderr, "    -h      display this help message and exit\n");
	exit(EX_USAGE);
}
