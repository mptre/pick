#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <err.h>
#include <locale.h>
#include <poll.h>
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

#define tty_putp(capability) do {					\
	if (tputs(capability, 1, tty_putc) == ERR)			\
		errx(1, #capability ": unknown terminfo capability");	\
	} while (0)

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
	LEFT,
	PAGE_DOWN,
	PAGE_UP
};

struct choice {
	char	*description;
	char	*string;
	size_t	 length;
	ssize_t	 match_start;	/* inclusive match start offset */
	ssize_t	 match_end;	/* exclusive match end offset */
	float	 score;
};

static int			 choicecmp(const void *, const void *);
static void			 delete_between(char *, size_t, size_t, size_t);
static char			*eager_strpbrk(const char *, const char *);
static void			 filter_choices(void);
static void			 get_choices(void);
static int			 get_key(char *, size_t, size_t *);
static void			 handle_sigint(int);
static int			 isu8cont(unsigned char);
static int			 isu8start(unsigned char);
static int			 min_match(const char *, size_t, ssize_t *,
				    ssize_t *);
static int			 print_choices(int, int);
static void			 print_line(const char *, size_t, int, ssize_t,
				    ssize_t);
static void			 score(struct choice *);
static const struct choice	*selected_choice(void);
static const char		*strcasechr(const char *, const char *);
static int			 tty_getc(void);
static void			 tty_init(void);
static int			 tty_putc(int);
static void			 tty_restore(void);
__dead static void		 usage(void);

static struct termios	 original_attributes;
static struct {
	size_t size;
	size_t length;
	struct choice *v;
}			 choices;
static FILE		*tty_in, *tty_out;
static char		*query;
static size_t		 query_length, query_size;
static int		 descriptions, choices_lines;
static int		 sort = 1;
static int		 use_alternate_screen = 1;

int
main(int argc, char **argv)
{
	const struct choice	*choice;
	int			 c;
	int			 output_description = 0;

#ifdef HAVE_PLEDGE
	if (pledge("stdio tty rpath wpath cpath", NULL) == -1)
		err(1, "pledge");
#endif

	setlocale(LC_CTYPE, "");

	while ((c = getopt(argc, argv, "dhoq:SvxX")) != -1)
		switch (c) {
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
			query_length = strlen(query);
			query_size = query_length + 1;
			break;
		case 'S':
			sort = 0;
			break;
		case 'v':
			puts(PACKAGE_VERSION);
			exit(EX_OK);
		case 'x':
			use_alternate_screen = 1;
			break;
		case 'X':
			use_alternate_screen = 0;
			break;
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (argc > 0)
		usage();

	if (query == NULL) {
		query_size = 64;
		if ((query = calloc(query_size, sizeof(char))) == NULL)
			err(1, NULL);
	}

	get_choices();
	tty_init();

#ifdef HAVE_PLEDGE
	if (pledge("stdio tty", NULL) == -1)
		err(1, "pledge");
#endif

	choice = selected_choice();
	tty_restore();
	if (choice != NULL) {
		printf("%s\n", choice->string);
		if (output_description)
			printf("%s\n", choice->description);
	}

	free(choices.v);
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

void
get_choices(void)
{
	char		*buf, *description, *field_separators, *start, *stop;
	ssize_t		 n;
	size_t		 length = 0;
	size_t		 size = BUFSIZ;

	if ((field_separators = getenv("IFS")) == NULL)
		field_separators = " ";

	if ((buf = malloc(size)) == NULL)
		err(1, NULL);
	for (;;) {
		if ((n = read(STDIN_FILENO, buf + length, size - length)) == -1)
			err(1, "read");
		else if (n == 0)
			break;

		length += n;
		if (length + 1 < size)
			continue;
		if ((buf = reallocarray(buf, 2, size)) == NULL)
			err(1, NULL);
		size *= 2;
	}
	memset(buf + length, '\0', size - length);

	choices.size = 16;
	if ((choices.v = reallocarray(NULL, choices.size,
			    sizeof(struct choice))) == NULL)
		err(1, NULL);

	start = buf;
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
		choices.v[choices.length].match_start = -1;
		choices.v[choices.length].match_end = -1;
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
	char	*tmp_ptr;
	char	*ptr = NULL;

	for (tmp_ptr = strpbrk(string, separators);
	     tmp_ptr;
	     tmp_ptr = strpbrk(tmp_ptr, separators))
		ptr = tmp_ptr++;

	return ptr;
}

const struct choice *
selected_choice(void)
{
	size_t	cursor_position, i, j, length;
	size_t	xscroll = 0;
	char	buf[6];
	int	choices_count, key, word_position;
	int	selection = 0;
	int	yscroll = 0;

	cursor_position = query_length;

	filter_choices();

	for (;;) {
		tty_putp(cursor_invisible);
		tty_putp(carriage_return);	/* move cursor to first column */
		if (cursor_position >= xscroll + columns)
			xscroll = cursor_position - columns + 1;
		if (cursor_position < xscroll)
			xscroll = cursor_position;
		print_line(&query[xscroll], query_length - xscroll, 0, -1, -1);
		choices_count = print_choices(yscroll, selection);
		if ((size_t)choices_count - yscroll < choices.length
		    && choices_count - yscroll < choices_lines) {
			/*
			 * Printing the choices did not consume all available
			 * lines and there could still be choices left from the
			 * last print in the lines not yet consumed.
			 *
			 * The clr_eos capability clears the screen from the
			 * current column to the end. If the last visible choice
			 * is selected, the standout in the last and current
			 * column will be also be cleared. Therefore, move down
			 * one line before clearing the screen.
			 */
			if (tty_putc('\n') == EOF)
				err(1, "tty_putc");
			tty_putp(clr_eos);
			tty_putp(tparm(parm_up_cursor, choices_count + 1));
		} else {
			tty_putp(tparm(parm_up_cursor,
				    choices_count < choices_lines
				    ? choices_count : choices_lines));
		}
		tty_putp(carriage_return);	/* move cursor to first column */
		for (i = j = 0; i < cursor_position; j++)
			while (isu8cont(query[++i]));
		if (j > 0)
			/*
			 * parm_right_cursor interprets 0 as 1, therefore only
			 * move the cursor if the position is non zero.
			 */
			tty_putp(tparm(parm_right_cursor, j));
		tty_putp(cursor_normal);
		fflush(tty_out);

		key = get_key(buf, sizeof(buf), &length);
		switch (key) {
		case ENTER:
			if (choices_count > 0)
				return &choices.v[selection];
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
				selection = yscroll = 0;
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
				selection = yscroll = 0;
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
			selection = yscroll = 0;
			break;
		case CTRL_K:
			delete_between(
			    query,
			    query_length,
			    cursor_position,
			    query_length);
			query_length = cursor_position;
			filter_choices();
			selection = yscroll = 0;
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
			selection = yscroll = 0;
			break;
		case CTRL_A:
			cursor_position = 0;
			break;
		case CTRL_E:
			cursor_position = query_length;
			break;
		case DOWN:
			if (selection < choices_count - 1) {
				selection++;
				if (selection - yscroll == choices_lines)
					yscroll++;
			}
			break;
		case UP:
			if (selection > 0) {
				selection--;
				if (selection - yscroll < 0)
					yscroll--;
			}
			break;
		case LEFT:
			while (cursor_position > 0
			    && isu8cont(query[--cursor_position]));
			break;
		case RIGHT:
			while (cursor_position < query_length
			    && isu8cont(query[++cursor_position]));
			break;
		case PAGE_DOWN:
			if (selection + choices_lines < choices_count)
				yscroll = selection += choices_lines;
			else
				selection = choices_count - 1;
			break;
		case PAGE_UP:
			if (selection - choices_lines > 0)
				yscroll = selection -= choices_lines;
			else
				yscroll = selection = 0;
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
			selection = yscroll = 0;
		}
	}
}

/*
 * Filter choices using the current query while there is no new user input
 * available.
 */
void
filter_choices(void)
{
	struct pollfd	pfd;
	size_t		i;
	int		nready;

	for (i = 0; i < choices.length; i++) {
		score(&choices.v[i]);

		/*
		 * Regularly check if there is any new user input available. If
		 * true, abort filtering since the currently used query is
		 * outdated. This improves the performance when the cardinality
		 * of the choices is large.
		 */
		if (i > 0 && i % 50 == 0) {
			pfd.fd = fileno(tty_in);
			pfd.events = POLLIN;
			if ((nready = poll(&pfd, 1, 0)) == -1)
				err(1, "poll");
			if (nready == 1 && pfd.revents & (POLLIN | POLLHUP))
				break;
		}
	}

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
	size_t	match_length;

	if (min_match(choice->string, 0,
		      &choice->match_start, &choice->match_end) == 0) {
		choice->match_start = choice->match_end = -1;
		choice->score = 0;
		return;
	}

	if (!sort) {
		choice->score = 1;
		return;
	}

	match_length = choice->match_end - choice->match_start;
	choice->score = (float)query_length / match_length / choice->length;
}

int
min_match(const char *string, size_t offset, ssize_t *start, ssize_t *end)
{
	const char	*e, *q, *s;
	size_t		 length;

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

	length = e - s;
	*start = s - string;
	*end = e - string;
	if (length > query_length && min_match(string, offset + 1, start, end)
	    && length < (size_t)(*end - *start)) {
		*start = s - string;
		*end = e - string;
	}
	return 1;
}

/*
 * Returns a pointer to first occurrence of the first character in s2 in s1 with
 * respect to Unicode characters and disregarding case.
 */
const char *
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
			return s1;
	}

	return NULL;
}

void
tty_init(void)
{
	struct termios	new_attributes;

	if ((tty_in = fopen("/dev/tty", "r")) == NULL)
		err(1, "fopen");

	tcgetattr(fileno(tty_in), &original_attributes);
	new_attributes = original_attributes;
	new_attributes.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(fileno(tty_in), TCSANOW, &new_attributes);

	if ((tty_out = fopen("/dev/tty", "w")) == NULL)
		err(1, "fopen");

	setupterm((char *)0, fileno(tty_out), (int *)0);

	choices_lines = lines - 1; /* available lines, minus query line */

	if (use_alternate_screen)
		tty_putp(enter_ca_mode);

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
	tty_restore();
	exit(EX_SIGINT);
}

void
tty_restore(void)
{
	tcsetattr(fileno(tty_in), TCSANOW, &original_attributes);
	fclose(tty_in);

	tty_putp(carriage_return);	/* move cursor to first column */
	tty_putp(clr_eos);

	if (use_alternate_screen)
		tty_putp(exit_ca_mode);

	fclose(tty_out);
}

void
print_line(const char *string, size_t length, int so, ssize_t ulso, ssize_t uleo)
{
	size_t	i;
	int	c, col, tabwidth;
	int	in_esc_seq = 0;
	int	non_printable = 0;

	if (so)
		tty_putp(enter_standout_mode);

	for (col = i = 0; i < length && col < columns; i++) {
		if (i == (size_t)ulso)
			tty_putp(enter_underline_mode);

		/*
		 * Count the number of outputted ANSI escape sequences
		 * in order to adjust the column count since they do not
		 * occupy any screen real-estate.
		 */
		if (in_esc_seq) {
			non_printable++;
			if (string[i] >= '@' && string[i] <= '~')
				in_esc_seq = 0;
		} else if (i > 0 && string[i - 1] == '\033'
		    && string[i] == '[') {
			in_esc_seq = 1;
			non_printable = 2;
		}

		c = string[i];
		if (c == '\t') {
			/* Ceil column count to multiple of 8. */
			col += tabwidth = 8 - (col & 7);
			while (tabwidth-- > 0)
				if (tty_putc(' ') == ERR)
					err(1, "tty_putc");
		} else {
			/*
			 * A null character will be present prior the
			 * terminating null character if descriptions is
			 * enabled.
			 */
			if (c == '\0')
				c = ' ';
			if (!isu8cont(c))
				col++;
			if (tty_putc(c) == EOF)
				err(1, "tty_putc");
		}

		if (i + 1 == (size_t)uleo)
			tty_putp(exit_underline_mode);
	}
	for (col -= non_printable; col < columns; col++)
		if (tty_putc(' ') == EOF)
			err(1, "tty_putc");

	if (so)
		tty_putp(exit_standout_mode);
}

/*
 * Output as many choices as possible starting from offset and return the number
 * of choices with a positive score. If the query is empty, all choices are
 * considered having a positive score.
 */
int
print_choices(int offset, int selection)
{
	struct choice	*choice;
	int		 i;

	for (i = offset, choice = &choices.v[i];
	     (size_t)i < choices.length
	     && (query_length == 0 || choice->score > 0);
	     choice++, i++) {
		if (i - offset < lines - 1)
			print_line(choice->string, choice->length,
			    i == selection, choice->match_start,
			    choice->match_end);
	}

	return i;
}

int
get_key(char *buf, size_t size, size_t *nread)
{
	static struct {
		const char	*s;
		size_t		 length;
		int		 key;
	}	keys[] = {
		{ "\n",		1,	ENTER },
		{ "\177",	1,	BACKSPACE },
		{ "\001",	1,	CTRL_A },
		{ "\002",	1,	LEFT },
		{ "\004",	1,	DEL },
		{ "\005",	1,	CTRL_E },
		{ "\006",	1,	RIGHT },
		{ "\013",	1,	CTRL_K },
		{ "\016",	1,	DOWN },
		{ "\020",	1,	UP },
		{ "\025",	1,	CTRL_U },
		{ "\027",	1,	CTRL_W },
		{ "\033\n",	2,	ALT_ENTER },
		{ "\033[A",	3,	UP },
		{ "\033OA",	3,	UP },
		{ "\033[B",	3,	DOWN },
		{ "\033OB",	3,	DOWN },
		{ "\033[C",	3,	RIGHT },
		{ "\033OC",	3,	RIGHT },
		{ "\033[D",	3,	LEFT },
		{ "\033OD",	3,	LEFT },
		{ "\033[3~",	4,	DEL },
		{ "\033O3~",	4,	DEL },
		{ "\033[6~",	4,	PAGE_DOWN },
		{ "\033[5~",	4,	PAGE_UP },
		{ NULL,		0,	0 },
	};
	int	c, i;

	*nread = 0;
getc:
	buf[(*nread)++] = tty_getc();
	size--;
	for (i = 0; keys[i].s != NULL; i++) {
		if (*nread > keys[i].length
		    || strncmp(buf, keys[i].s, *nread) != 0)
			continue;

		if (*nread == keys[i].length)
			return keys[i].key;

		/* Partial match found, continue reading. */
		if (size > 0)
			goto getc;
	}

	if (*nread > 1 && buf[0] == '\033' && (buf[1] == '[' || buf[1] == 'O')) {
		/*
		 * A escape sequence which is not a supported key is being read.
		 * Discard the rest of the sequence.
		 */
		for (;;) {
			c = tty_getc();
			if (c >= '@' && c <= '~')
				break;
		}

		return UNKNOWN;
	}

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
