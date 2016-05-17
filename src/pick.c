#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/ttydefaults.h>
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
#include <wchar.h>

#ifdef HAVE_NCURSESW_H
#include <ncursesw/curses.h>
#include <ncursesw/term.h>
#else
#include <curses.h>
#include <term.h>
#endif

#include "compat.h"

#define EX_SIG 128
#define EX_SIGINT (EX_SIG + SIGINT)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define tty_putp(capability) do { \
	if (tputs(capability, 1, tty_putc) == ERR) \
		errx(1, #capability ": unknown terminfo capability"); \
	} while (0)

#define ESCAPE 27

enum {
	UNKNOWN,
	ALT_ENTER,
	BACKSPACE,
	DEL,
	ENTER,
	CTRL_A,
	CTRL_E,
	CTRL_K,
	CTRL_U,
	CTRL_W,
	UP,
	RIGHT,
	DOWN,
	LEFT
};

struct choice {
	char	*description;
	char	*string;
	size_t	 length;
	size_t	 printable_length;
	size_t	 match_start;
	size_t	 match_end;
	float	 score;
};

__dead static void		 usage(void);
__dead static void		 version(void);
static void			 get_choices(void);
static char			*eager_strpbrk(const char *, const char *);
static size_t			 count_printable(char *, size_t length);
static void			 put_choice(const struct choice *);
static const struct choice	*selected_choice(void);
static void			 filter_choices(void);
static void			 score(struct choice *);
static int			 min_match(const char *, size_t, size_t *, size_t *);
static char			*strcasechr(const char *, const char *);
static void			 init_tty(void);
static int			 tty_putc(int);
static void			 handle_sigint(int);
static void			 restore_tty(void);
static void			 put_line(char *, int);
static int			 print_choices(int);
static int			 get_key(char *, size_t, size_t *);
static int			 tty_getc(void);
static void			 delete_between(char *, size_t, size_t, size_t);
static void			 print_query(char *, size_t, size_t, size_t);
static int			 choicecmp(const void *, const void *);
static int			 isu8cont(unsigned char);
static int			 isu8start(unsigned char);

static FILE		*tty_in;
static FILE		*tty_out;
static char		*query;
static int		 descriptions;
static int		 output_description;
static int		 use_alternate_screen;
static int		 sort = 1;
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
	const struct choice	*choice;
	int			 option;

	use_alternate_screen = getenv("VIM") == NULL;

	setlocale(LC_CTYPE, "");

	while ((option = getopt(argc, argv, "dhoq:SvxX")) != -1) {
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
		case 'S':
			sort = 0;
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

		if ((query = calloc(query_size, sizeof(char))) == NULL)
			err(1, NULL);
	}

	get_choices();
	init_tty();
	choice = selected_choice();
	restore_tty();
	if (choice != NULL)
		put_choice(choice);

	free(choices.v);
	free(input.string);
	free(query);

	return EX_OK;
}

__dead void
usage(void)
{
	fprintf(stderr, "usage: pick [-hvS] [-d [-o]] [-x | -X] [-q query]\n"
	    "    -h          output this help message and exit\n"
	    "    -v          output the version and exit\n"
	    "    -S          disable sorting\n"
	    "    -d          read and display descriptions\n"
	    "    -o          output description of selected on exit\n"
	    "    -x          enable alternate screen\n"
	    "    -X          disable alternate screen\n"
	    "    -q query    supply an initial search query\n");

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
		err(1, NULL);
	for (;;) {
		if ((length = read(STDIN_FILENO, input.string + input.length,
				   input.size - input.length)) <= 0)
			break;

		input.length += length;
		if (input.length + 1 < input.size)
			continue;
		input.size *= 2;
		if ((input.string = realloc(input.string, input.size)) == NULL)
			err(1, NULL);
	}
	memset(input.string + input.length, '\0', input.size - input.length);

	choices.size = 16;
	if ((choices.v = reallocarray(NULL, choices.size,
			    sizeof(struct choice))) == NULL)
		err(1, NULL);

	start = input.string;
	while ((stop = strchr(start, '\n')) != NULL) {
		*stop = '\0';

		if (descriptions &&
		    (description = eager_strpbrk(start, field_separators)))
			*description++ = '\0';
		else
			description = "";

		choices.v[choices.length].length = stop - start;
		choices.v[choices.length].string = start;
		choices.v[choices.length].description = description;
		choices.v[choices.length].printable_length =
		    count_printable(start, stop - start);
		choices.v[choices.length].match_start = 0;
		choices.v[choices.length].match_end = 0;
		choices.v[choices.length].score = 0;

		start = stop + 1;

		/* Ensure room for a extra choice when ALT_ENTER is invoked. */
		if (++choices.length + 1 < choices.size)
			continue;
		choices.size *= 2;
		if ((choices.v = reallocarray(choices.v, choices.size,
				    sizeof(struct choice))) == NULL)
			err(1, NULL);
	}
}

char *
eager_strpbrk(const char *string, const char *separators)
{
	char	*ptr = NULL, *tmp_ptr;

	for (tmp_ptr = strpbrk(string, separators);
	     tmp_ptr;
	     tmp_ptr = strpbrk(tmp_ptr, separators))
		ptr = tmp_ptr++;

	return ptr;
}

size_t
count_printable(char *str, size_t length)
{
	int	in_esc_seq = 0, printable = 0;
	size_t	i;

	for (i = 0; i < length; i++) {
		if (in_esc_seq) {
			in_esc_seq = (str[i - 1] == ESCAPE && str[i] == '[') ||
			    str[i] < '@' || str[i] > '~';
		} else {
			in_esc_seq = str[i] == ESCAPE;
			if (!in_esc_seq)
				printable++;
		}
	}
	return printable;
}

void
put_choice(const struct choice *choice)
{
	puts(choice->string);

	if (output_description)
		puts(choice->description);
}

const struct choice *
selected_choice(void)
{
	char		 buf[6];
	int		 key, selection = 0, visible_choices_count;
	int		 word_position;
	size_t		 cursor_position, length, query_length, scroll;

	cursor_position = query_length = strlen(query);

	filter_choices();

	if (cursor_position >= (size_t)columns)
		scroll = cursor_position - columns + 1;
	else
		scroll = 0;

	visible_choices_count = print_choices(selection);
	print_query(query, query_length, cursor_position, scroll);
	tty_putp(cursor_normal);

	for (;;) {
		fflush(tty_out);
		memset(buf, 0, sizeof(buf));
		key = get_key(buf, sizeof(buf), &length);

		switch (key) {
		case ENTER:
			if (visible_choices_count > 0) {
				if (selection >= 0 && selection < (ssize_t)choices.length)
					return &choices.v[selection];
				else
					return NULL;
			}

			break;
		case ALT_ENTER:
			choices.v[choices.length].string = query;
			choices.v[choices.length].description = "";
			return &choices.v[choices.length];
		case BACKSPACE:
			if (cursor_position > 0) {
				for (length = 1;
				    isu8cont(query[cursor_position - length]);
				    length++);
				delete_between(
				    query,
				    query_length,
				    cursor_position - length,
				    cursor_position);
				cursor_position -= length;
				query_length -= length;
				filter_choices();
				selection = 0;
			}

			break;
		case DEL:
			if (cursor_position < query_length) {
				for (length = 1;
				    isu8cont(query[cursor_position + length]);
				    length++);
				delete_between(
				    query,
				    query_length,
				    cursor_position,
				    cursor_position + length);
				query_length -= length;
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
			    cursor_position,
			    query_length);
			query_length = cursor_position;
			filter_choices();
			selection = 0;
			break;
		case CTRL_W:
			if (cursor_position == 0)
				break;

			for (word_position = cursor_position;;) {
				while (isu8cont(query[--word_position]));
				if (word_position < 1)
					break;
				if (query[word_position] != ' '
				    && query[word_position - 1] == ' ')
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
			while (cursor_position > 0
			    && isu8cont(query[--cursor_position]));
			break;
		case RIGHT:
			while (cursor_position < query_length
			    && isu8cont(query[++cursor_position]));
			break;
		default:
			if (!isu8start(buf[0]) && !isprint(buf[0]))
				continue;

			if (query_size < query_length + length) {
				query_size = 2*query_length + length;
				if ((query = reallocarray(query, query_size,
					    sizeof(char))) == NULL)
					err(1, NULL);
			}

			if (cursor_position < query_length)
				memmove(query + cursor_position + length,
					query + cursor_position,
					query_length - cursor_position);

			memcpy(query + cursor_position, buf, length);
			cursor_position += length;
			query_length += length;
			query[query_length] = '\0';
			filter_choices();
			selection = 0;

			break;
		}

		tty_putp(cursor_invisible);

		visible_choices_count = print_choices(selection);
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
		score(&choices.v[i]);

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

void
score(struct choice *choice)
{
	size_t	query_length, match_length;

	if (min_match(choice->string, 0,
		      &choice->match_start, &choice->match_end) == 0) {
		choice->match_start = choice->match_end = choice->score = 0;
		return;
	}

	if (!sort) {
		choice->score = 1;
		return;
	}

	match_length = choice->match_end - choice->match_start;
	query_length = strlen(query);
	choice->score = (float)query_length / match_length / choice->length;
}

int
min_match(const char *string, size_t offset, size_t *start, size_t *end)
{
	char	 *s, *e, *q;

	q = query;
	if ((s = e = strcasechr(&string[offset], q)) == NULL)
		return 0;

	for (;;) {
		for (e++, q++; isu8cont(*q); e++, q++);
		if (*q == '\0')
			break;
		if ((e = strcasechr(e, q)) == NULL)
			return 0;
	}

	*start = s - string;
	*end = e - string;
	/* Less than or equal is used in order to obtain the left-most match. */
	if (min_match(string, offset + 1, start, end)
	    && (size_t)(e - s) <= *end - *start) {
		*start = s - string;
		*end = e - string;
	}
	return 1;
}

/*
 * Returns a pointer to first occurrence of the first character in s2 in s1 with
 * respect to Unicode characters and disregarding case.
 */
char *
strcasechr(const char *s1, const char *s2)
{
	wchar_t	 wc1, wc2;

	switch (mbtowc(&wc2, s2, MB_CUR_MAX)) {
	case -1:
		mbtowc(NULL, NULL, MB_CUR_MAX);
		/* FALLTHROUGH */
	case 0:
		return NULL;
	}

	for (; *s1; s1++) {
		if (mbtowc(&wc1, s1, MB_CUR_MAX) == -1) {
			mbtowc(NULL, NULL, MB_CUR_MAX);
			continue;
		}
		if (wcsncasecmp(&wc1, &wc2, 1) == 0)
			return (char *)s1;
	}

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
	for (i = 0; i < (ssize_t)choices.length && i < lines - 1; ++i)
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
put_line(char *string, int length)
{
	int	i;

	for (i = 0; i < length && i < columns; ++i) {
		if (tty_putc(string[i]) == EOF)
			err(1, "tty_putc");
	}
	for (; i < columns; ++i) {
		if (tty_putc(' ') == EOF)
			err(1, "tty_putc");
	}
}

void
print_query(char *string, size_t length, size_t position, size_t scroll)
{
	size_t	i = 0;

	tty_putp(restore_cursor);
	put_line(string + scroll, length - scroll);

	tty_putp(restore_cursor);
	while (i < position - scroll) {
		while (isu8cont(string[++i]));
		tty_putp(cursor_right);
	}
}

int
print_choices(int selection)
{
	int		 col, i, j, k;
	size_t		 query_length;
	struct choice	*choice;

	tty_putp(clr_eos);
	/* Emit query line. */
	tty_putc('\n');

	query_length = strlen(query);
	for (choice = choices.v, col = i = 0;
	     i < (ssize_t)choices.length
	     && i < lines - 1
	     && (query_length == 0 || choice->score > 0);
	     choice++, i++) {
		if (i == selection)
			tty_putp(enter_standout_mode);

		for (col = j = 0;
		     j < (ssize_t)choice->match_start && col < columns;
		     col += !isu8cont(choice->string[j]), j++)
			if (tty_putc(choice->string[j]) == EOF)
				err(1, "tty_putc");

		tty_putp(enter_underline_mode);

		for (;
		     j < (ssize_t)choice->match_end && col < columns;
		     col += !isu8cont(choice->string[j]), j++)
			if (tty_putc(choice->string[j]) == EOF)
				err(1, "tty_putc");

		tty_putp(exit_underline_mode);

		for (;
		     j < (ssize_t)choice->length && col < columns;
		     col += !isu8cont(choice->string[j]), j++) {
			/* A null character will be present before the
			 * terminating null character if descriptions is
			 * enabled. */
			if (choice->string[j] == '\0') {
				if (tty_putc(' ') == EOF)
					err(1, "tty_putc");
			} else if (tty_putc(choice->string[j]) == EOF) {
				err(1, "tty_putc");
			}
		}

		for (k = MAX(columns - choice->printable_length +
			    (choice->length - col), 0); k > 0; k--)
			if (tty_putc(' ') == EOF)
				err(1, "tty_putc");

		if (i == selection)
			tty_putp(exit_standout_mode);
	}

	return i;
}

int
get_key(char *buf, size_t size, size_t *nread)
{
	static struct {
		union {
			const char *s;
			char c;
		} input;
		size_t length;
		int key;
	} keys[] = {
		{ { (char *)10 },		1,	ENTER },
		{ { (char *)127 },		1,	BACKSPACE },
		{ { (char *)CTRL('A') },	1,	CTRL_A },
		{ { (char *)CTRL('B') },	1,	LEFT },
		{ { (char *)CTRL('D') },	1,	DEL },
		{ { (char *)CTRL('E') },	1,	CTRL_E },
		{ { (char *)CTRL('F') },	1,	RIGHT },
		{ { (char *)CTRL('K') },	1,	CTRL_K },
		{ { (char *)CTRL('N') },	1,	DOWN },
		{ { (char *)CTRL('P') },	1,	UP },
		{ { (char *)CTRL('U') },	1,	CTRL_U },
		{ { (char *)CTRL('W') },	1,	CTRL_W },
		{ { "\033\n" },			2,	ALT_ENTER },
		{ { "\033[A" },			3,	UP },
		{ { "\033OA" },			3,	UP },
		{ { "\033[B" },			3,	DOWN },
		{ { "\033OB" },			3,	DOWN },
		{ { "\033[C" },			3,	RIGHT },
		{ { "\033OC" },			3,	RIGHT },
		{ { "\033[D" },			3,	LEFT },
		{ { "\033OD" },			3,	LEFT },
		{ { "\033[3~" },		4,	DEL },
		{ { "\033O3~" },		4,	DEL },
		{ { NULL },			0,	0 },
	};
	const char	*key;
	int		 i;

	*nread = 0;
getc:
	buf[(*nread)++] = tty_getc();
	size--;
	for (i = 0; keys[i].input.s; i++) {
		key = keys[i].length > 1 ? keys[i].input.s : &keys[i].input.c;
		if (*nread > keys[i].length || strncmp(buf, key, *nread))
			continue;

		if (*nread == keys[i].length)
			return keys[i].key;

		/* Partial match found, continue reading. */
		if (size > 0)
			goto getc;
	}

	if (*nread > 1 && buf[0] == '\033' && (buf[1] == '[' || buf[1] == 'O'))
		/*
		 * A escape sequence which is not a supported key is being read.
		 * Ensure the whole sequence is read.
		 */
		while ((buf[*nread - 1] < '@' || buf[*nread - 1] > '~')
		    && size-- > 0)
			buf[(*nread)++] = tty_getc();

	if (!isu8start(buf[0]))
		return UNKNOWN;

	/*
	 * Ensure a whole Unicode character is read. The number of MSBs in the
	 * first octet of a Unicode character is equal to the number of octets
	 * the character consists of, followed by a zero. Therefore, as long as
	 * the MSB is not zero there is still bytes left to read.
	 */
	while (((buf[0] << *nread) & 0x80) == 0x80 && size-- > 0)
		buf[(*nread)++] = tty_getc();

	return UNKNOWN;
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

int
isu8cont(unsigned char c)
{
	return (c & (0x80 | 0x40)) == 0x80;
}

int
isu8start(unsigned char c)
{
	return (c & (0x80 | 0x40)) == (0x80 | 0x40);
}
