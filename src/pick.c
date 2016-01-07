#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <termios.h>
#include <unistd.h>

#ifdef HAVE_NCURSESW_H
#include <ncursesw/curses.h>
#include <ncursesw/term.h>
#else
#include <curses.h>
#include <term.h>
#endif

#include "compat.h"

#define DEL 127
#define ENTER 10
#define ALT_ENTER 266
#define BACKSPACE 263
#define UP 259
#define DOWN 258
#define RIGHT 261
#define LEFT 260
#define ESCAPE 27

#define EX_SIG 128
#define EX_SIGINT (EX_SIG + SIGINT)

#define tty_putp(capability) do { \
	if (tputs(capability, 1, tty_putc) == ERR) \
		errx(1, #capability ": unknown terminfo capability"); \
	} while (0)

struct choice {
	char	*description;
	char	*string;
	float	 score;
};

__dead static void	 usage(void);
__dead static void	 version(void);
static void 		 get_choices(void);
static char		*eager_strpbrk(const char *, const char *);
static void		 put_choice(struct choice *);
static struct choice	*selected_choice(void);
static void		 filter_choices(void);
static float		 score(char *);
static size_t		 min_match_length(char *);
static char		*strcasechr(const char *, char);
static void		 init_tty(void);
static int		 tty_putc(int);
static void		 handle_sigint(int);
static void		 restore_tty(void);
static void		 put_line(char *, int, int);
static int		 print_choices(int);
static int		 get_key(void);
static int		 tty_getc(void);
static void		 delete_between(char *, size_t, size_t, size_t);
static void		 free_choices(void);
static void		 print_query(char *, size_t, size_t, size_t);
static int		 choicecmp(const void *, const void *);

static FILE		*tty_in;
static FILE		*tty_out;
static char		*query = NULL;
static int		 descriptions = 0;
static int		 output_description = 0;
static int		 use_alternate_screen;
static size_t		 query_size;
static struct termios	 original_attributes;
static struct {
	size_t size;
	size_t length;
	struct choice *v;
} choices;
static struct {
	size_t	size;
	size_t	length;
	char	*string;
} input;

int
main(int argc, char **argv)
{
	int	option;

	use_alternate_screen = getenv("VIM") == NULL;

	while ((option = getopt(argc, argv, "dhoq:vxX")) != -1) {
		switch (option) {
		case 'd':
			descriptions = 1;
			break;
		case 'o':
			/*
			 * Only output description if descriptions are read and
			 * displayed in the list of choices.
			 */
			output_description = descriptions;
			break;
		case 'q':
			if ((query = strdup(optarg)) == NULL)
				err(1, "strdup");
			query_size = strlen(query) + 1;
			break;
		case 'v':
			version();
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

	if (query == NULL) {
	    query_size = 64;

	    if ((query = calloc(query_size, sizeof(*query))) == NULL)
		    err(1, "calloc");
	}

	get_choices();
	put_choice(selected_choice());

	free_choices();
	free(query);

	return EX_OK;
}

__dead void
usage(void)
{
	extern char	*__progname;

	fprintf(stderr, "usage: %s [-hv] [-d [-o]] [-x | -X] [-q query]\n"
	    "    -h          output this help message and exit\n"
	    "    -v          output the version and exit\n"
	    "    -d          read and display descriptions\n"
	    "    -o          output description of selected on exit\n"
	    "    -x          enable alternate screen\n"
	    "    -X          disable alternate screen\n"
	    "    -q query    supply an initial search query\n", __progname);

	exit(EX_USAGE);
}

__dead void
version(void)
{
	puts(PACKAGE_VERSION);
	exit(EX_OK);
}

void
get_choices(void)
{
	char		*description, *field_separators, *start, *stop;
	ssize_t		 length;

	if ((field_separators = getenv("IFS")) == NULL)
		field_separators = " ";

	input.size = BUFSIZ;
	if ((input.string = malloc(input.size)) == NULL)
		err(1, "malloc");
	for (;;) {
		if ((length = read(STDIN_FILENO, input.string + input.length,
				   input.size - input.length)) <= 0)
		    break;

		input.length += length;
		if (input.length + 1 < input.size)
			continue;
		input.size *= 2;
		if ((input.string = realloc(input.string, input.size)) == NULL)
			err(1, "realloc");
	}
	memset(input.string + input.length, '\0', input.size - input.length);

	choices.size = 16;
	if ((choices.v = malloc(choices.size*sizeof(struct choice))) == NULL)
		err(1, "malloc");

	start = input.string;
	while ((stop = strchr(start, '\n')) != NULL) {
		*stop = '\0';

		if (descriptions &&
		    (description = eager_strpbrk(start, field_separators)))
			*description++ = '\0';
		else
			description = "";

		choices.v[choices.length].string = start;
		choices.v[choices.length].description = description;
		choices.v[choices.length].score = 0;

		start = stop + 1;

		/* Ensure room for a extra choice when ALT_ENTER is invoked. */
		if (++choices.length + 1 < choices.size)
			continue;
		choices.size *= 2;
		if ((choices.v = realloc(choices.v, choices.size*sizeof(struct choice))) == NULL)
			err(1, "realloc");
	}
}

char *
eager_strpbrk(const char *string, const char *separators) {
	char	*ptr = NULL, *tmp_ptr;

	for (tmp_ptr = strpbrk(string, separators);
	     tmp_ptr;
	     tmp_ptr = strpbrk(tmp_ptr, separators))
		ptr = tmp_ptr++;

	return ptr;
}

void
put_choice(struct choice *choice)
{
	puts(choice->string);

	if (output_description)
		puts(choice->description);
}

struct choice *
selected_choice(void)
{
	int		 key, selection = 0, visible_choices_count;
	int		 word_position;
	size_t		 cursor_position, query_length, scroll;

	cursor_position = query_length = strlen(query);

	filter_choices();
	init_tty();

	if (cursor_position >= (size_t)columns)
		scroll = cursor_position - columns + 1;
	else
		scroll = 0;

	visible_choices_count = print_choices(selection);
	print_query(query, query_length, cursor_position, scroll);
	tty_putp(cursor_normal);

	for (;;) {
		fflush(tty_out);
		key = get_key();

		switch(key) {
		case ENTER:
			if (visible_choices_count > 0) {
				restore_tty();
				if (selection >= 0 && selection < (ssize_t)choices.length)
					return &choices.v[selection];
				else
					return NULL;
			}

			break;
		case ALT_ENTER:
			restore_tty();
			choices.v[choices.length].string = query;
			choices.v[choices.length].description = "";
			return &choices.v[choices.length];
		case BACKSPACE:
		case DEL:
			if (cursor_position > 0) {
				delete_between(
				    query,
				    query_length,
				    cursor_position - 1,
				    cursor_position);
				--cursor_position;
				--query_length;
				filter_choices();
				selection = 0;
			}

			break;
		case CTRL('D'):
			if (cursor_position < query_length) {
				delete_between(
				    query,
				    query_length,
				    cursor_position,
				    cursor_position + 1);
				--query_length;
				filter_choices();
				selection = 0;
			}

			break;
		case CTRL('U'):
			delete_between(
			    query,
			    query_length,
			    0,
			    cursor_position);
			query_length -= cursor_position;
			cursor_position = 0;
			filter_choices();
			selection = 0;
			break;
		case CTRL('K'):
			delete_between(
			    query,
			    query_length,
			    cursor_position,
			    query_length);
			query_length = cursor_position;
			filter_choices();
			selection = 0;
			break;
		case CTRL('W'):
			if (cursor_position > 0) {
				for (word_position = cursor_position - 1;
				    word_position > 0;
				    --word_position) {
					if (query[word_position] != ' ' &&
					    query[word_position - 1] == ' ')
						break;
				}

				delete_between(
				    query,
				    query_length,
				    word_position,
				    cursor_position);
				query_length -= cursor_position - word_position;
				cursor_position = word_position;
				filter_choices();
				selection = 0;
			}
			break;
		case CTRL('A'):
			cursor_position = 0;
			break;
		case CTRL('E'):
			cursor_position = query_length;
			break;
		case CTRL('N'):
		case DOWN:
			if (selection < visible_choices_count - 1)
				++selection;
			break;
		case CTRL('P'):
		case UP:
			if (selection > 0)
				--selection;
			break;
		case CTRL('B'):
		case LEFT:
			if (cursor_position > 0)
				--cursor_position;
			break;
		case CTRL('F'):
		case RIGHT:
			if (cursor_position < query_length)
				++cursor_position;
			break;
		default:
			if (key > 31 && key < 127) { /* Printable chars */
				if (cursor_position < query_length) {
					memmove(
					    query + cursor_position + 1,
					    query + cursor_position,
					    query_length - cursor_position);
				}

				query[cursor_position++] = key;
				query[++query_length] = '\0';
				filter_choices();
				selection = 0;
			}

			break;
		}

		tty_putp(cursor_invisible);

		if (query_length == query_size - 1) {
			query_size += query_size;

			query = reallocarray(query, query_size, sizeof(*query));
			if (query == NULL)
				err(1, "reallocarray");
		}

		visible_choices_count = print_choices(selection);
		tty_putp(clr_eos);
		if (cursor_position >= scroll + columns)
			scroll = cursor_position - columns + 1;
		if (cursor_position < scroll)
			scroll = cursor_position;
		print_query(query, query_length, cursor_position, scroll);
		tty_putp(cursor_normal);
	}
}

void
filter_choices(void)
{
	int	i;

	for (i = 0; i < (ssize_t)choices.length; ++i)
		choices.v[i].score = score(choices.v[i].string);

	qsort(choices.v, choices.length, sizeof(struct choice), choicecmp);
}

int
choicecmp(const void *p1, const void *p2)
{
	const struct choice	*c1, *c2;

	c1 = p1, c2 = p2;
	if (c1->score < c2->score)
		return 1;
	if (c1->score > c2->score)
		return -1;
	return c1->string - c2->string;
}

float
score(char *string)
{
	size_t	string_length, query_length, match_length;

	if ((match_length = min_match_length(string)) == INT_MAX)
		return 0;

	string_length = strlen(string);
	query_length = strlen(query);

	return (float)query_length / match_length / string_length;
}

size_t
min_match_length(char *string)
{
	char	*start, *end;
	int	 i;
	size_t	 length, min;

	if (!(start = strcasechr(string, query[0])))
		return INT_MAX;
	for (i = 1, end = start + 1; *end && query[i]; end++)
		if (strncasecmp(end, query + i, 1) == 0)
			i++;
	if (query[i])
		return INT_MAX;
	length = end - start;
	min = min_match_length(string + 1);

	return length < min ? length : min;
}

static char *
strcasechr(const char *s, char c)
{
	for(; *s && c; s++)
		if (strncasecmp(s, &c, 1) == 0)
			return (char *)s;
	return NULL;
}

void
init_tty(void)
{
	struct termios	 new_attributes;
	int		 i;

	if ((tty_in = fopen("/dev/tty", "r")) == NULL) {
		err(1, "fopen");
	}

	tcgetattr(fileno(tty_in), &original_attributes);
	new_attributes = original_attributes;
	new_attributes.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(fileno(tty_in), TCSANOW, &new_attributes);

	if ((tty_out = fopen("/dev/tty", "w")) == NULL)
		err(1, "fopen");

	setupterm((char *)0, fileno(tty_out), (int *)0);

	if (use_alternate_screen)
		tty_putp(enter_ca_mode);

	/* Emit enough lines to fit all choices. */
	for (i = 0; i < (ssize_t)choices.length && i < lines; ++i)
		tty_putp(cursor_down);
	for (; i > 0; --i)
		tty_putp(cursor_up);
	tty_putp(save_cursor);

	signal(SIGINT, handle_sigint);
}

int
tty_putc(int c)
{
    return putc(c, tty_out);
}

void
handle_sigint(int sig __attribute__((unused)))
{
	restore_tty();
	exit(EX_SIGINT);
}

void
restore_tty(void)
{
	tcsetattr(fileno(tty_in), TCSANOW, &original_attributes);
	fclose(tty_in);

	tty_putp(restore_cursor);
	tty_putp(clr_eos);

	if (use_alternate_screen)
		tty_putp(exit_ca_mode);

	fclose(tty_out);
}

void
put_line(char *string, int length, int standout)
{
	int	i;

	if (standout)
		tty_putp(enter_standout_mode);

	for (i = 0; string[i] != '\0' && i < columns; ++i) {
		if (tty_putc(string[i]) == EOF)
			err(1, "tty_putc");
	}
	for (; i < columns; ++i) {
		if (tty_putc(' ') == EOF)
			err(1, "tty_putc");
	}

	if (standout)
		tty_putp(exit_standout_mode);
}

void
print_query(char *query, size_t length, size_t position, size_t scroll)
{
	size_t	i;

	tty_putp(restore_cursor);
	put_line(query + scroll, length - scroll, 0);

	tty_putp(restore_cursor);
	for (i = 0; i < position - scroll; ++i)
		tty_putp(cursor_right);
}

int
print_choices(int selection)
{
	char		*line;
	int		 i;
	size_t		 length, line_length = 64, query_length;
	struct choice	*choice;

	/* Emit query line. */
	tty_putc('\n');

	if ((line = calloc(sizeof(*line), line_length)) == NULL)
		err(1, "calloc");

	query_length = strlen(query);
	for (choice = choices.v, i = 0;
	     i < (ssize_t)choices.length
	     && i < lines - 1
	     && (query_length == 0 || choice->score > 0);
	     choice++, i++) {
		length = strlen(choice->string) + strlen(choice->description) +
		    1;

		while (length > line_length) {
			line_length = line_length * 2;

			if ((line = reallocarray(line, line_length,
					    sizeof(*line))) == NULL)
				err(1, "reallocarray");
		}

		strlcpy(line, choice->string, line_length);
		strlcat(line, " ", line_length);
		strlcat(line, choice->description, line_length);

		put_line(line, length, i == selection);
	}

	free(line);

	return i;
}

int
get_key(void)
{
	int	key;

	key = tty_getc();

	if (key == ESCAPE) {
		key = tty_getc();

		if (key == '\n') {
			return ALT_ENTER;
		}

		if (key == '[' || key == 'O') {
			key = tty_getc();
			switch (key) {
			case 'A':
				return UP;
			case 'B':
				return DOWN;
			case 'C':
				return RIGHT;
			case 'D':
				return LEFT;
			}
		}
	}

	return key;
}

int
tty_getc(void)
{
	int	c;

	if ((c = getc(tty_in)) == ERR)
		err(1, "getc");

	return c;
}

void
delete_between(char *string, size_t length, size_t start, size_t end)
{
	memmove(string + start, string + end, length - end + 1);
}

void
free_choices(void)
{
	free(choices.v);
	free(input.string);
}
