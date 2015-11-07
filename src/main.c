#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_FULL_QUEUE_H
#include <sys/queue.h>
#endif

#define _WITH_GETLINE

#include <ctype.h>
#include <err.h>
#include <fcntl.h>
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

#define TTY_CTRL_A 1
#define TTY_CTRL_B 2
#define TTY_CTRL_D 4
#define TTY_CTRL_E 5
#define TTY_CTRL_F 6
#define TTY_CTRL_N 14
#define TTY_CTRL_P 16
#define TTY_CTRL_K 11
#define TTY_CTRL_U 21
#define TTY_CTRL_W 23
#define TTY_DEL 127
#define TTY_ENTER 10
#define TTY_ALT_ENTER 266
#define TTY_BACKSPACE 263
#define TTY_UP 259
#define TTY_DOWN 258
#define TTY_RIGHT 261
#define TTY_LEFT 260

#define EX_SIG 128
#define EX_SIGINT (EX_SIG + SIGINT)

#define ESCAPE 27

struct choice {
	char *string;
	char *description;
	float score;
	SLIST_ENTRY(choice) choices;
};

SLIST_HEAD(choices, choice);

static void	free_choices(void);
static size_t min_match_length(char *);
static float score(char *);
static struct choice *merge(struct choice *, struct choice *);
static struct choice *sort(struct choice *);
static struct choice	*new_choice(char *, char *, float);
static void		 free_choice(struct choice *);
static void 		 get_choices(void);
static void		 put_choice(struct choice *);
static void chomp(char *, ssize_t);
static char * eager_strpbrk(const char *, const char *);
static struct choice	*selected_choice(void);
static void put_line(int, char *, int, int);
static int print_choices(int);
static struct choice *choice_at(int);
static void filter_choices(int *);
static void delete_between(char *, size_t, size_t, size_t);
static void print_string_at(int, int, char *, int);
static void tty_init(void);
static void tty_restore();
static int tty_getch();
static void tty_putc(int);
static void tty_show_cursor();
static void tty_hide_cursor();
static void tty_enter_standout_mode();
static void tty_exit_standout_mode();
static void tty_move_cursor_to(int, int);
static int tty_getc();
static void tty_putp(const char *);
static int raw_tty_putc(int);
static void handle_sigint();
static void usage();
static void version();

static FILE *tty_out;
static FILE *tty_in;
static struct termios original_attributes;
static int use_alternate_screen;
static int descriptions = 0;
static int output_description = 0;
static struct choices *choices;
static char *query = NULL;
static size_t query_size;

int
main(int argc, char **argv)
{
	int option;

	use_alternate_screen = getenv("VIM") == NULL;

	while ((option = getopt(argc, argv, "hvdoq:xX")) != -1) {
		switch (option) {
		case 'v':
			version();
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

	free(query);
	free_choices();

	return EX_OK;
}

static void
free_choices(void)
{
	struct choice *choice;

	while (!SLIST_EMPTY(choices)) {
		choice = SLIST_FIRST(choices);
		SLIST_REMOVE_HEAD(choices, choices);
		free_choice(choice);
	}

	free(choices);
}

static size_t
min_match_length(char *string)
{
	size_t match_length, match_start, query_position, match_position;
	int query_char, query_start;

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
					if (string[match_position] == '\0') {
						return match_length;
					}

					if (tolower((unsigned char)string[match_position]) == query_char) {
						++match_position;
						break;
					}
				}
			}
			if (match_length == 0 || match_length > match_position -
			    match_start + 1) {
				match_length = match_position - match_start + 1;
			}
		}
	}

	return match_length;
}

static float
score(char *string)
{
	size_t string_length, query_length, match_length;

	string_length = strlen(string);
	query_length = strlen(query);

	if (query_length == 0) {
		return 1;	
	}

	if (string_length == 0) {
		return 0;
	}

	match_length = min_match_length(string);
	if (match_length == 0) {
		return 0;
	}

	return (float)query_length / (float)match_length / (float)string_length;
}

static struct choice *
merge(struct choice *front, struct choice *back)
{
	struct choice head;
	struct choice *choice;

	choice = &head;

	while (front != NULL && back != NULL) {
		if (front->score > back->score ||
		    (front->score == back->score &&
		     strcmp(front->string, back->string) < 0)) {
			choice->choices.sle_next = front;
			choice = front;
			front = front->choices.sle_next;
		} else {
			choice->choices.sle_next = back;
			choice = back;
			back = back->choices.sle_next;
		}
	}

	if (front != NULL) {
		choice->choices.sle_next = front;
	} else {
		choice->choices.sle_next = back;
	}

	return head.choices.sle_next;
}

static struct choice *
sort(struct choice *choice)
{
	struct choice *front, *back;

	if (choice == NULL || choice->choices.sle_next == NULL) {
		return choice;
	}

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
new_choice(char *string, char *description, float score)
{
	struct choice *choice;

	choice = malloc(sizeof(struct choice));
	if (choice == NULL) {
		err(1, "malloc");
	}

	choice->string = strdup(string);
	choice->description = strdup(description);
	choice->score = score;

	return choice;
}

static void
free_choice(struct choice *choice)
{
	free(choice->string);
	free(choice->description);
	free(choice);
}

void
get_choices(void)
{
	char *line, *description, *field_separators;
	size_t line_size;
	ssize_t length;
	struct choice *choice;

	field_separators = getenv("IFS");
	if (field_separators == NULL) {
		field_separators = " ";
	}

	choices = malloc(sizeof(struct choices));
	if (choices == NULL) {
		err(1, "malloc");
	}

	SLIST_INIT(choices);

	for (;;) {
		line = NULL;
		line_size = 0;

		length = getline(&line, &line_size, stdin);
		if (length == -1) {
			break;
		}

		chomp(line, length);

		if (descriptions && (description = eager_strpbrk(line, field_separators))) {
			*description++ = '\0';
		} else {
			description = "";
		}

		choice = new_choice(line, description, 1);
		SLIST_INSERT_HEAD(choices, choice, choices);

		free(line);
	}

	free(line);
}

static void
put_choice(struct choice *choice)
{
	printf("%s\n", choice->string);

	if (output_description) {
		printf("%s\n", choice->description);
	}
}

static void
chomp(char *string, ssize_t length)
{
	if (string[length - 1] == '\n') {
		string[length - 1] = '\0';
	}
}

static char *
eager_strpbrk(const char *string, const char *separators) {
	char *ptr = NULL, *tmp_ptr;
	for (tmp_ptr = strpbrk(string, separators);
	     tmp_ptr;
	     tmp_ptr = strpbrk(tmp_ptr, separators)) {
		ptr = tmp_ptr++;
	}
	return ptr;
}

static struct choice *
selected_choice(void)
{
	int key, selection, visible_choices_count, word_position;
	size_t cursor_position, query_length;
	struct choice *choice;

	query_length = strlen(query);
	cursor_position = query_length;

	filter_choices(&selection);
	tty_init();

	put_line(0, query, query_length, 0);
	visible_choices_count = print_choices(selection);
	tty_move_cursor_to(0, cursor_position);
	tty_show_cursor();

	for (;;) {
		fflush(tty_out);
		key = tty_getch();
		switch(key) {
		case TTY_ENTER:
			if (visible_choices_count > 0) {
				tty_restore();
				return choice_at(selection);
			}

			break;
		case TTY_ALT_ENTER:
			tty_restore();
			choice = new_choice(query, "", 1);
			SLIST_INSERT_HEAD(choices, choice, choices);
			return choice;
		case TTY_CTRL_N:
			if (selection < visible_choices_count - 1) {
				++selection;
			}

			break;
		case TTY_CTRL_P:
			if (selection > 0) {
				--selection;
			}

			break;
		case TTY_CTRL_B:
			if (cursor_position > 0) {
				--cursor_position;
			}

			break;
		case TTY_CTRL_F:
			if (cursor_position < query_length) {
				++cursor_position;
			}

			break;
		case TTY_BACKSPACE:
		case TTY_DEL:
			if (cursor_position > 0) {
				delete_between(
				    query,
				    query_length,
				    cursor_position - 1,
				    cursor_position);
				--cursor_position;
				--query_length;
				filter_choices(&selection);
			}

			break;
		case TTY_CTRL_D:
			if (cursor_position < query_length) {
				delete_between(
				    query,
				    query_length,
				    cursor_position,
				    cursor_position + 1);
				--query_length;
				filter_choices(&selection);
			}

			break;
		case TTY_CTRL_U:
			delete_between(
			    query,
			    query_length,
			    0,
			    cursor_position);
			query_length -= cursor_position;
			cursor_position = 0;
			filter_choices(&selection);
			break;
		case TTY_CTRL_K:
			delete_between(
			    query,
			    query_length,
			    cursor_position + 1,
			    query_length);
			query_length = cursor_position;
			filter_choices(&selection);
			break;
		case TTY_CTRL_W:
			if (cursor_position > 0) {
				for (word_position = cursor_position - 1;
				    word_position > 0;
				    --word_position) {
					if (query[word_position] != ' ' &&
					    query[word_position - 1] == ' ') {
						break;
					}
				}

				delete_between(
				    query,
				    query_length,
				    word_position,
				    cursor_position);
				query_length -= cursor_position - word_position;
				cursor_position = word_position;
				filter_choices(&selection);
			}
			break;
		case TTY_CTRL_A:
			cursor_position = 0;
			break;
		case TTY_CTRL_E:
			cursor_position = query_length;
			break;
		case TTY_DOWN:
			if (selection < visible_choices_count - 1) {
				++selection;
			}

			break;
		case TTY_UP:
			if (selection > 0) {
				--selection;
			}

			break;
		case TTY_LEFT:
			if (cursor_position > 0) {
				--cursor_position;
			}

			break;
		case TTY_RIGHT:
			if (cursor_position < query_length) {
				++cursor_position;
			}

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
				filter_choices(&selection);
			}

			break;
		}

		tty_hide_cursor();

		if (query_length == query_size - 1) {
			query_size += query_size;

			query = reallocarray(query, query_size, sizeof(*query));
			if (query == NULL) {
				err(1, "reallocarray");
			}
		}

		put_line(0, query, query_length, 0);
		visible_choices_count = print_choices(selection);
		tty_move_cursor_to(0, cursor_position);
		tty_show_cursor();
	}
}

static void
put_line(int y, char *string, int length, int standout)
{
	if (standout) {
		tty_enter_standout_mode();
	}

	if (length > 0) {
		print_string_at(y, 0, string, columns);
	}

	tty_move_cursor_to(y, length);

	for (; length < columns; ++length) {
		tty_putc(' ');
	}

	if (standout) {
		tty_exit_standout_mode();
	}
}

static int
print_choices(int selection)
{
	char *line;
	int i;
	int visible_choices_count = 0;
	size_t length;
	size_t line_length = 64;
	struct choice *choice;

	line = calloc(sizeof(*line), line_length);
	if (line == NULL) {
		err(1, "calloc");
	}

	SLIST_FOREACH(choice, choices, choices) {
		length = strlen(choice->string) + strlen(choice->description) +
		    1;

		while (length > line_length) {
			line_length = line_length * 2;

			line = reallocarray(line, line_length, sizeof(*line));
			if (line == NULL) {
				err(1, "reallocarray");
			}
		}

		strlcpy(line, choice->string, line_length);
		strlcat(line, " ", line_length);
		strlcat(line, choice->description, line_length);

		if (visible_choices_count == lines - 1 || choice->score == 0) {
			break;
		}

		put_line(visible_choices_count + 1, line, length,
		    visible_choices_count == selection);

		++visible_choices_count;
	}

	free(line);

	for (i = visible_choices_count + 1; i < lines; ++i) {
		put_line(i, "", 0, 0);
	}

	return visible_choices_count;
}

static struct choice *
choice_at(int index)
{
	struct choice *choice;
	int i = 0;

	SLIST_FOREACH(choice, choices, choices) {
		if (i == index) {
			return choice;
		}

		++i;
	}

	return NULL;
}

static void
filter_choices(int *selection)
{
	struct choice *choice;

	SLIST_FOREACH(choice, choices, choices) {
		choice->score = score(choice->string);
	}

	choices->slh_first = sort(choices->slh_first);

	*selection = 0;
}

static void
delete_between(char *string, size_t length, size_t start, size_t end)
{
	memmove(string + start, string + end, length - end + 1);
}

static void
print_string_at(int y, int x, char *string, int max_length)
{
	int i;

	tty_move_cursor_to(y, x);

	for (i = 0; string[i] != '\0' && i < max_length; i++) {
		tty_putc(string[i]);
	}
}

static void
tty_init(void)
{
	struct termios new_attributes;

	tty_in = fopen("/dev/tty", "r");
	if (tty_in == NULL) {
		err(1, "fopen");
	}

	tcgetattr(fileno(tty_in), &original_attributes);
	new_attributes = original_attributes;
	new_attributes.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(fileno(tty_in), TCSANOW, &new_attributes);

	tty_out = fopen("/dev/tty", "w");
	if (tty_out == NULL) {
		err(1, "fopen");
	}

	setupterm((char *)0, fileno(tty_out), (int *)0);

	if (use_alternate_screen) {
		tty_putp(enter_ca_mode);
	}

	tty_putp(clear_screen);

	signal(SIGINT, handle_sigint);
}

static void
tty_restore()
{
	tcsetattr(fileno(tty_in), TCSANOW, &original_attributes);
	fclose(tty_in);

	tty_putp(clear_screen);

	if (use_alternate_screen) {
		tty_putp(exit_ca_mode);
	}

	fclose(tty_out);
}

static int
tty_getch()
{
	int ch;

	ch = tty_getc();

	if (ch == ESCAPE) {
		ch = tty_getc();

		if (ch == '\n') {
			return TTY_ALT_ENTER;
		}

		if (ch == '[' || ch == 'O') {
			ch = tty_getc();

			if (ch == 'A') {
				return TTY_UP;
			}

			if (ch == 'B') {
				return TTY_DOWN;
			}

			if (ch == 'C') {
				return TTY_RIGHT;
			}

			if (ch == 'D') {
				return TTY_LEFT;
			}
		}
	}

	return ch;
}

static void
tty_putc(int c)
{
	if (raw_tty_putc(c) == EOF) {
		err(1, "raw_tty_putc");
	}
}

static void
tty_show_cursor()
{
	tty_putp(cursor_normal);
}

static void
tty_hide_cursor()
{
	tty_putp(cursor_invisible);
}

static void
tty_enter_standout_mode()
{
	tty_putp(enter_standout_mode);
}

static void
tty_exit_standout_mode()
{
	tty_putp(exit_standout_mode);
}

static void
tty_move_cursor_to(int y, int x)
{
	tty_putp(tgoto(cursor_address, x, y));
}

static int
tty_getc()
{
	int c;

	c = getc(tty_in);

	if (c == ERR) {
		err(1, "getc");
	}

	return c;
}

static void
tty_putp(const char *string)
{
	if (tputs(string, 1, raw_tty_putc) == ERR) {
		err(1, "tputs");
	}
}

static int
raw_tty_putc(int c)
{
    return putc(c, tty_out);
}

static void
handle_sigint()
{
	tty_restore();
	exit(EX_SIGINT);
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
