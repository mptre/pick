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
	int display_desc;
	int output_desc;
	int use_alternate_screen;
	char *query;
	struct choices *cs;

	display_desc = 0;
	output_desc = 0;
	use_alternate_screen = 1;
	if (getenv("VIM") != NULL) {
		use_alternate_screen = 0;
	}
	query = "";
	while ((ch = getopt(argc, argv, "hvdoq:X")) != -1)
		switch (ch) {
		case 'v':
			version();
		case 'd':
			display_desc = 1;
			break;
		case 'o':
			output_desc = 1;
			break;
		case 'q':
			query = optarg;
			break;
		case 'X':
			use_alternate_screen = 0;
			break;
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	output_desc = output_desc && display_desc;

	cs = get_choices(display_desc);
	put_choice(get_selected(cs, query, use_alternate_screen), output_desc);
	choices_free(cs);
	return EX_OK;
}

void
usage()
{
	fprintf(stderr, "usage: pick [-h] [-v] [-q QUERY] [-d [-o]] \n");
	fprintf(stderr, "    -h          output this help message and exit\n");
	fprintf(stderr, "    -v          output the version and exit\n");
	fprintf(stderr, "    -q QUERY    supply an initial search query\n");
	fprintf(stderr, "    -d          read and display descriptions\n");
	fprintf(stderr, "    -o          output description of selected on exit\n");
	exit(EX_USAGE);
}

void
version()
{
	printf("%s\n", PACKAGE_VERSION);
	exit(EX_OK);
}
