#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_FULL_QUEUE_H
#include <sys/queue.h>
#endif

#include <ctype.h>
#include <err.h>
#include <locale.h>
#include <signal.h>
#define _WITH_GETLINE
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

#define CTRL_A 1
#define CTRL_B 2
#define CTRL_D 4
#define CTRL_E 5
#define CTRL_F 6
#define CTRL_N 14
#define CTRL_P 16
#define CTRL_K 11
#define CTRL_U 21
#define CTRL_W 23
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
	SLIST_ENTRY(choice)	 choices;
	char			*description;
	char			*string;
	float			 score;
};

SLIST_HEAD(choices, choice);

__dead static void	 usage(void);
__dead static void	 version(void);
static void 		 get_choices(void);
static char		*eager_strpbrk(const char *, const char *);
static struct choice	*new_choice(char *, char *, float);
static void		 put_choice(struct choice *);
static struct choice	*selected_choice(void);
static void		 filter_choices(void);
static float		 score(char *);
static size_t		 min_match_length(char *);
static struct choice	*sort(struct choice *);
static struct choice	*merge(struct choice *, struct choice *);
static void		 init_tty(void);
static int		 tty_putc(int);
static void		 handle_sigint(int);
static void		 restore_tty(void);
static void		 put_line(char *, int, int);
static void		 start_standout(void);
static void		 end_standout(void);
static int		 print_choices(int);
static void		 show_cursor(void);
static int		 get_key(void);
static int		 tty_getc(void);
static struct choice	*choice_at(int);
static void		 delete_between(char *, size_t, size_t, size_t);
static void		 hide_cursor(void);
static void		 free_choices(void);
static void		 print_query(char *, size_t, int);

static FILE		*tty_in;
static FILE		*tty_out;
static char		*query = NULL;
static int		 descriptions = 0;
static int		 output_description = 0;
static int		 use_alternate_screen;
static size_t		 query_size;
static struct termios	 original_attributes;
static struct choices	*choices;
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

__dead static void
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

__dead static void
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
	struct choice	*choice;

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

	if ((choices = malloc(sizeof(struct choices))) == NULL)
		err(1, "malloc");

	SLIST_INIT(choices);

	start = input.string;
	while ((stop = strchr(start, '\n')) != NULL) {
		*stop = '\0';

		if (descriptions &&
		    (description = eager_strpbrk(start, field_separators)))
			*description++ = '\0';
		else
			description = "";

		choice = new_choice(start, description, 1);
		SLIST_INSERT_HEAD(choices, choice, choices);

		start = stop + 1;
	}
}

static char *
eager_strpbrk(const char *string, const char *separators) {
	char	*ptr = NULL, *tmp_ptr;

	for (tmp_ptr = strpbrk(string, separators);
	     tmp_ptr;
	     tmp_ptr = strpbrk(tmp_ptr, separators))
		ptr = tmp_ptr++;

	return ptr;
}

static struct choice *
new_choice(char *string, char *description, float score)
{
	struct choice	*choice;

	if ((choice = malloc(sizeof(struct choice))) == NULL)
		err(1, "malloc");

	choice->string = string;
	choice->description = description;
	choice->score = score;

	return choice;
}

static void
put_choice(struct choice *choice)
{
	puts(choice->string);

	if (output_description)
		puts(choice->description);
}

static struct choice *
selected_choice(void)
{
	int		 key, selection, visible_choices_count, word_position;
	size_t		 cursor_position, query_length;
	struct choice	*choice;

	query_length = strlen(query);
	cursor_position = query_length;

	filter_choices();
	selection = 0;

	init_tty();

	visible_choices_count = print_choices(selection);
	print_query(query, query_length, cursor_position);
	show_cursor();

	for (;;) {
		fflush(tty_out);
		key = get_key();

		switch(key) {
		case ENTER:
			if (visible_choices_count > 0) {
				restore_tty();
				return choice_at(selection);
			}

			break;
		case ALT_ENTER:
			restore_tty();
			choice = new_choice(query, "", 1);
			SLIST_INSERT_HEAD(choices, choice, choices);
			return choice;
		case CTRL_N:
			if (selection < visible_choices_count - 1)
				++selection;

			break;
		case CTRL_P:
			if (selection > 0)
				--selection;

			break;
		case CTRL_B:
			if (cursor_position > 0)
				--cursor_position;

			break;
		case CTRL_F:
			if (cursor_position < query_length)
				++cursor_position;

			break;
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
		case CTRL_D:
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
		case CTRL_U:
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
		case CTRL_K:
			delete_between(
			    query,
			    query_length,
			    cursor_position + 1,
			    query_length);
			query_length = cursor_position;
			filter_choices();
			selection = 0;
			break;
		case CTRL_W:
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
		case CTRL_A:
			cursor_position = 0;
			break;
		case CTRL_E:
			cursor_position = query_length;
			break;
		case DOWN:
			if (selection < visible_choices_count - 1)
				++selection;
			break;
		case UP:
			if (selection > 0)
				--selection;
			break;
		case LEFT:
			if (cursor_position > 0)
				--cursor_position;
			break;
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

		hide_cursor();

		if (query_length == query_size - 1) {
			query_size += query_size;

			query = reallocarray(query, query_size, sizeof(*query));
			if (query == NULL)
				err(1, "reallocarray");
		}

		visible_choices_count = print_choices(selection);
		tty_putp(clr_eos);
		print_query(query, query_length, cursor_position);
		show_cursor();
	}
}

static void
filter_choices(void)
{
	struct choice	*choice;

	SLIST_FOREACH(choice, choices, choices) {
		choice->score = score(choice->string);
	}

	choices->slh_first = sort(choices->slh_first);
}

static float
score(char *string)
{
	size_t	string_length, query_length, match_length;

	string_length = strlen(string);
	query_length = strlen(query);

	if (query_length == 0)
		return 1;

	if (string_length == 0)
		return 0;

	if ((match_length = min_match_length(string)) == 0)
		return 0;

	return (float)query_length / (float)match_length / (float)string_length;
}

static size_t
min_match_length(char *string)
{
	size_t	match_length, match_start, query_position, match_position;
	int	query_char, query_start;

	query_start = tolower((unsigned char)query[0]);

	for (match_length = 0, match_start = 0; string[match_start] != '\0';
	    ++match_start) {
		if (tolower((unsigned char)string[match_start]) ==
		    query_start) {
			for (query_position = 1,
			    match_position = match_start + 1;
			    query[query_position] != '\0'; ++query_position) {
				query_char = tolower(
				    (unsigned char)query[query_position]);

				for (;; ++match_position) {
					if (string[match_position] == '\0')
						return match_length;

					if (tolower((unsigned char)string[match_position]) == query_char) {
						++match_position;
						break;
					}
				}
			}
			if (match_length == 0 || match_length > match_position -
			    match_start + 1)
				match_length = match_position - match_start + 1;
		}
	}

	return match_length;
}

static struct choice *
sort(struct choice *choice)
{
	struct choice	*front, *back;

	if (choice == NULL || choice->choices.sle_next == NULL)
		return choice;

	front = choice;
	back = choice->choices.sle_next;

	while (back != NULL && back->choices.sle_next != NULL) {
		choice = choice->choices.sle_next;
		back = back->choices.sle_next->choices.sle_next;
	}

	back = choice->choices.sle_next;
	choice->choices.sle_next = NULL;

	return merge(sort(front), sort(back));
}

static struct choice *
merge(struct choice *front, struct choice *back)
{
	struct choice	head, *choice;

	choice = &head;

	while (front != NULL && back != NULL) {
		if (front->score > back->score ||
		    (front->score == back->score &&
		     front->string < back->string)) {
			choice->choices.sle_next = front;
			choice = front;
			front = front->choices.sle_next;
		} else {
			choice->choices.sle_next = back;
			choice = back;
			back = back->choices.sle_next;
		}
	}

	if (front != NULL)
		choice->choices.sle_next = front;
	else
		choice->choices.sle_next = back;

	return head.choices.sle_next;
}

static void
init_tty(void)
{
	struct termios	 new_attributes;
	struct choice	*choice;
	int		 choices_count = 0;

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
	SLIST_FOREACH(choice, choices, choices) {
		choices_count++;
		tty_putp(cursor_down);
		if (choices_count == lines)
			break;
	}
	while (choices_count-- > 0)
		tty_putp(cursor_up);
	tty_putp(save_cursor);

	signal(SIGINT, handle_sigint);
}

static int
tty_putc(int c)
{
    return putc(c, tty_out);
}

static void
handle_sigint(int sig __attribute__((unused)))
{
	restore_tty();
	exit(EX_SIGINT);
}

static void
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

static void
put_line(char *string, int length, int standout)
{
	int	i;

	if (standout)
		start_standout();

	for (i = 0; string[i] != '\0' && i < columns; ++i) {
		if (tty_putc(string[i]) == EOF)
			err(1, "tty_putc");
	}
	for (; i < columns; ++i) {
		if (tty_putc(' ') == EOF)
			err(1, "tty_putc");
	}

	if (standout)
		end_standout();
}

static void
start_standout(void)
{
	tty_putp(enter_standout_mode);
}

static void
end_standout(void)
{
	tty_putp(exit_standout_mode);
}

static void
print_query(char *query, size_t length, int position)
{
	int	i;

	tty_putp(restore_cursor);
	put_line(query, length, 0);

	tty_putp(restore_cursor);
	for (i = 0; i < position; ++i)
		tty_putp(cursor_right);
}

static int
print_choices(int selection)
{
	char		*line;
	int		 visible_choices_count = 0;
	size_t		 length, line_length = 64;
	struct choice	*choice;

	/* Emit query line. */
	tty_putc('\n');

	if ((line = calloc(sizeof(*line), line_length)) == NULL)
		err(1, "calloc");

	SLIST_FOREACH(choice, choices, choices) {
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

		if (visible_choices_count == lines - 1 || choice->score == 0)
			break;

		put_line(line, length, visible_choices_count == selection);

		++visible_choices_count;
	}

	free(line);

	return visible_choices_count;
}

static void
show_cursor(void)
{
	tty_putp(cursor_normal);
}

static int
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

			if (key == 'A') {
				return UP;
			}

			if (key == 'B') {
				return DOWN;
			}

			if (key == 'C') {
				return RIGHT;
			}

			if (key == 'D') {
				return LEFT;
			}
		}
	}

	return key;
}

static int
tty_getc(void)
{
	int	c;

	if ((c = getc(tty_in)) == ERR)
		err(1, "getc");

	return c;
}

static struct choice *
choice_at(int index)
{
	struct choice	*choice;
	int		 i = 0;

	SLIST_FOREACH(choice, choices, choices) {
		if (i == index)
			return choice;
		++i;
	}

	return NULL;
}

static void
delete_between(char *string, size_t length, size_t start, size_t end)
{
	memmove(string + start, string + end, length - end + 1);
}

static void
hide_cursor(void)
{
	tty_putp(cursor_invisible);
}

static void
free_choices(void)
{
	struct choice	*choice;

	while (!SLIST_EMPTY(choices)) {
		choice = SLIST_FIRST(choices);
		SLIST_REMOVE_HEAD(choices, choices);

		free(choice);
	}

	free(choices);
	free(input.string);
}
