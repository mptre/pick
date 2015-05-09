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
	int option, use_alternate_screen;
	int display_descriptions = 0;
	int output_description = 0;
	struct choices *choices;

	use_alternate_screen = getenv("VIM") == NULL;

	while ((option = getopt(argc, argv, "hvdoq:xX")) != -1) {
		switch (option) {
		case 'v':
			version();
		case 'd':
			display_descriptions = 1;
			break;
		case 'o':
			output_description = 1;
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
	 * Only output description if descriptions are read and displayed in the
	 * list of choices.
	 */
	output_description = output_description && display_descriptions;

	choices = io_read_choices(display_descriptions);

	io_print_choice(
	    ui_selected_choice(choices, query, use_alternate_screen),
	    output_description);

	choices_free(choices);

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
