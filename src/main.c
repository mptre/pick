#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#include "choices.h"
#include "config.h"
#include "io.h"
#include "ui.h"

static void usage();
static void version();

int
main(int argc, char **argv)
{
	char *query = "";
	int ch, use_alternate_screen;
	int display_desc = 0;
	int output_desc = 0;
	struct choices *cs;

	use_alternate_screen = getenv("VIM") == NULL;

	while ((ch = getopt(argc, argv, "hvdoq:xX")) != -1) {
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
		case 'x':
			use_alternate_screen = 1;
			break;
		case 'X':
			use_alternate_screen = 0;
			break;
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	/*
	 * Only output descriptions if descriptions are read and displayed in
	 * the list of choices.
	 */
	output_desc = output_desc && display_desc;

	cs = get_choices(display_desc);

	put_choice(get_selected(cs, query, use_alternate_screen), output_desc);

	choices_free(cs);

	return EX_OK;
}

static void
usage()
{
	fprintf(stderr,
	    "usage: pick [-h] [-v] [-q QUERY] [-d [-o]] [-x | -X]\n");
	fprintf(stderr, "    -h          output this help message and exit\n");
	fprintf(stderr, "    -v          output the version and exit\n");
	fprintf(stderr, "    -q QUERY    supply an initial search query\n");
	fprintf(stderr, "    -d          read and display descriptions\n");
	fprintf(stderr,
	    "    -o          output description of selected on exit\n");
	fprintf(stderr, "    -x          enable alternate screen\n");
	fprintf(stderr, "    -X          disable alternate screen\n");

	exit(EX_USAGE);
}

static void
version()
{
	printf("%s\n", PACKAGE_VERSION);

	exit(EX_OK);
}
