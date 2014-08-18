#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#include "choices.h"
#include "config.h"
#include "io.h"
#include "ui.h"

void usage();
void version();

int
main(int argc,char **argv)
{
	int ch;
	struct choices *cs;

	while ((ch = getopt(argc, argv, "hv")) != -1)
		switch (ch) {
		case 'v':
			version();
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
	fprintf(stderr, "usage: pick [-h] [-v]\n");
	fprintf(stderr, "    -h      display this help message and exit\n");
	fprintf(stderr, "    -v      display the version and exit\n");
	exit(EX_USAGE);
}

void
version()
{
	printf("%s\n", PACKAGE_VERSION);
	exit(EX_OK);
}
