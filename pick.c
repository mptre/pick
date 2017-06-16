#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <locale.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define tty_putp(capability, fatal) do {				\
	if (tputs(capability, 1, tty_putc) == ERR && fatal)		\
		errx(1, #capability ": unknown terminfo capability");	\
} while (0)

enum key {
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
	PAGE_UP,
	HOME,
	END,
	PRINTABLE
};

struct choice {
	const char	*description;
	const char	*string;
	size_t		 length;
	ssize_t		 match_start;	/* inclusive match start offset */
	ssize_t		 match_end;	/* exclusive match end offset */
	double		 score;
};

static int			 choicecmp(const void *, const void *);
static void			 delete_between(char *, size_t, size_t, size_t);
static char			*eager_strpbrk(const char *, const char *);
static void			 filter_choices(void);
static char			*get_choices(void);
static enum key			 get_key(char *, size_t, size_t *);
static void			 handle_sigint(int);
static int			 isu8cont(unsigned char);
static int			 isu8start(unsigned char);
static size_t			 min_match(const char *, size_t, ssize_t *,
				    ssize_t *);
static int			 print_choices(int, int);
static void			 print_line(const char *, size_t, int, ssize_t,
				    ssize_t);
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
	char			*input;
	int			 c;
	int			 output_description = 0;

	setlocale(LC_CTYPE, "");

#ifdef HAVE_PLEDGE
	if (pledge("stdio tty rpath wpath cpath", NULL) == -1)
		err(1, "pledge");
#endif

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
			exit(0);
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

	input = get_choices();
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

	free(input);
	free(choices.v);
	free(query);

	return 0;
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

	exit(1);
}

char *
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

	return buf;
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
	int	choices_count, word_position;
	int	selection = 0;
	int	yscroll = 0;

	cursor_position = query_length;

	filter_choices();

	for (;;) {
		tty_putp(cursor_invisible, 0);
		tty_putp(carriage_return, 1);	/* move cursor to first column */
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
			tty_putp(clr_eos, 1);
			tty_putp(tparm(parm_up_cursor, choices_count + 1), 1);
		} else {
			tty_putp(tparm(parm_up_cursor,
				    choices_count < choices_lines
				    ? choices_count : choices_lines), 1);
		}
		tty_putp(carriage_return, 1);	/* move cursor to first column */
		for (i = j = 0; i < cursor_position; j++)
			while (isu8cont(query[++i]))
				continue;
		if (j > 0)
			/*
			 * parm_right_cursor interprets 0 as 1, therefore only
			 * move the cursor if the position is non zero.
			 */
			tty_putp(tparm(parm_right_cursor, j), 1);
		tty_putp(cursor_normal, 0);
		fflush(tty_out);

		switch (get_key(buf, sizeof(buf), &length)) {
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
				    length++)
					continue;
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
				    length++)
					continue;
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
				while (isu8cont(query[--word_position]))
					continue;
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
			    && isu8cont(query[--cursor_position]))
				continue;
			break;
		case RIGHT:
			while (cursor_position < query_length
			    && isu8cont(query[++cursor_position]))
				continue;
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
		case HOME:
			if (selection > 0)
				yscroll = selection = 0;
			break;
		case END:
			if (selection < choices_count - 1)
				selection = choices_count - 1;
				if (selection - yscroll >= choices_lines)
					yscroll = selection - choices_lines + 1;
			break;
		case PRINTABLE:
			if (query_length + length >= query_size) {
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
		case UNKNOWN:
			break;
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
	struct choice	*c;
	struct pollfd	 pfd;
	size_t		 i, match_length;
	int		 nready;

	for (i = 0; i < choices.length; i++) {
		c = &choices.v[i];
		if (min_match(c->string, 0,
			    &c->match_start, &c->match_end) == INT_MAX) {
			c->match_start = c->match_end = -1;
			c->score = 0;
		} else if (!sort) {
			c->score = 1;
		} else {
			match_length = c->match_end - c->match_start;
			c->score = (float)query_length/match_length/c->length;
		}

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

size_t
min_match(const char *string, size_t offset, ssize_t *start, ssize_t *end)
{
	const char	*e, *q, *s;
	size_t		 length;

	q = query;
	if ((s = e = strcasechr(&string[offset], q)) == NULL)
		return INT_MAX;

	for (;;) {
		for (e++, q++; isu8cont(*q); e++, q++)
			continue;
		if (*q == '\0')
			break;
		if ((e = strcasechr(e, q)) == NULL)
			return INT_MAX;
	}

	length = e - s;
	/* LEQ is used to obtain the shortest left-most match. */
	if (length == query_length
	    || length <= min_match(string, s - string + 1, start, end)) {
		*start = s - string;
		*end = e - string;
	}

	return length;
}

/*
 * Returns a pointer to first occurrence of the first character in s2 in s1 with
 * respect to Unicode characters, ANSI escape sequences and disregarding case.
 */
const char *
strcasechr(const char *s1, const char *s2)
{
	wchar_t	wc1, wc2;
	size_t	i;
	int	in_esc_seq, nbytes;

	switch (mbtowc(&wc2, s2, MB_CUR_MAX)) {
	case -1:
		mbtowc(NULL, NULL, MB_CUR_MAX);
		/* FALLTHROUGH */
	case 0:
		return NULL;
	}

	in_esc_seq = 0;
	for (i = 0; s1[i] != '\0';) {
		nbytes = 1;

		if (in_esc_seq) {
			if (s1[i] >= '@' && s1[i] <= '~')
				in_esc_seq = 0;
		} else if (i > 0 && s1[i - 1] == '\033' && s1[i] == '[') {
			in_esc_seq = 1;
		} else if ((nbytes = mbtowc(&wc1, &s1[i], MB_CUR_MAX)) == -1) {
			mbtowc(NULL, NULL, MB_CUR_MAX);
		} else if (wcsncasecmp(&wc1, &wc2, 1) == 0) {
			return &s1[i];
		}

		if (nbytes > 0)
			i += nbytes;
		else
			i++;
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

	choices_lines = lines - 1;	/* available lines, minus query line */

	if (use_alternate_screen)
		tty_putp(enter_ca_mode, 0);

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
	exit(1);
}

void
tty_restore(void)
{
	tcsetattr(fileno(tty_in), TCSANOW, &original_attributes);
	fclose(tty_in);

	tty_putp(carriage_return, 1);	/* move cursor to first column */
	tty_putp(clr_eos, 1);

	if (use_alternate_screen)
		tty_putp(exit_ca_mode, 0);

	fclose(tty_out);
}

void
print_line(const char *str, size_t len, int standout,
    ssize_t enter_underline, ssize_t exit_underline)
{
	size_t	i;
	wchar_t	wc;
	int	col, in_esc_seq, nbytes, width;

	if (standout)
		tty_putp(enter_standout_mode, 1);

	col = i = in_esc_seq = 0;
	while (col < columns) {
		if (enter_underline == (ssize_t)i)
			tty_putp(enter_underline_mode, 1);
		else if (exit_underline == (ssize_t)i)
			tty_putp(exit_underline_mode, 1);
		if (i == len)
			break;

		if (str[i] == '\t') {
			width = 8 - (col & 7);	/* ceil to multiple of 8 */
			if (col + width > columns)
				break;
			col += width;

			for (; width > 0; width--)
				if (tty_putc(' ') == ERR)
					err(1, "tty_putc");

			i++;
			continue;
		}

		/*
		 * A NUL will be present prior the NUL-terminator if
		 * descriptions are enabled.
		 */
		if (str[i] == '\0') {
			if (tty_putc(' ') == ERR)
				err(1, "tty_putc");

			i++, col++;
			continue;
		}

		/*
		 * Due to the explicit NUL-check above, the case where
		 * mbtowc(3) returns 0 is not handled here.
		 */
		if ((nbytes = mbtowc(&wc, &str[i], MB_CUR_MAX)) == -1) {
			mbtowc(NULL, NULL, MB_CUR_MAX);
			i++;
			continue;
		}

		width = 0;
		if (i > 0 && str[i - 1] == '\033' && str[i] == '[')
			/*
			 * Start of ANSI escape sequence. The previous
			 * ESC-character already has a zero width but any
			 * following characters will not consume any columns
			 * once displayed.
			 */
			in_esc_seq = 1;
		else if (!in_esc_seq && (width = wcwidth(wc)) < 0)
			/*
			 * The character is not printable. However, it could be
			 * an ESC-character marking the the beginning an escape
			 * sequence so make sure to display every valid
			 * characters.
			 */
			width = 0;
		else if (str[i] >= '@' && str[i] <= '~')
			in_esc_seq = 0;

		if (col + width > columns)
			break;
		col += width;

		for (; nbytes > 0; nbytes--, i++)
			if (tty_putc(str[i]) == EOF)
				err(1, "tty_putc");
	}
	for (; col < columns; col++)
		if (tty_putc(' ') == EOF)
			err(1, "tty_putc");

	/*
	 * If exit_underline is greater than columns the underline attribute
	 * will spill over on the next line unless all attributes are exited.
	 */
	tty_putp(exit_attribute_mode, 1);
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
		if (i - offset < choices_lines)
			print_line(choice->string, choice->length,
			    i == selection, choice->match_start,
			    choice->match_end);
	}

	return i;
}

enum key
get_key(char *buf, size_t size, size_t *nread)
{
#define	KEY(k, s)	{ k, s, sizeof(s) - 1 }
	static struct {
		enum key	 key;
		const char	*s;
		size_t		 length;
	}	keys[] = {
		KEY(ALT_ENTER,	"\033\n"),
		KEY(ALT_ENTER,	"\033\r"),
		KEY(BACKSPACE,	"\b"),
		KEY(BACKSPACE,	"\177"),
		KEY(CTRL_A,	"\001"),
		KEY(CTRL_E,	"\005"),
		KEY(CTRL_K,	"\013"),
		KEY(CTRL_U,	"\025"),
		KEY(CTRL_W,	"\027"),
		KEY(DEL,	"\004"),
		KEY(DEL,	"\033O3~"),
		KEY(DEL,	"\033[3~"),
		KEY(DOWN,	"\016"),
		KEY(DOWN,	"\033OB"),
		KEY(DOWN,	"\033[B"),
		KEY(ENTER,	"\n"),
		KEY(ENTER,	"\r"),
		KEY(LEFT,	"\002"),
		KEY(LEFT,	"\033OD"),
		KEY(LEFT,	"\033[D"),
		KEY(PAGE_DOWN,	"\033[6~"),
		KEY(PAGE_UP,	"\033[5~"),
		KEY(HOME,	"\033[7~"),
		KEY(HOME,	"\033[OH"),
		KEY(END,	"\033[8~"),
		KEY(END,	"\033[OF"),
		KEY(RIGHT,	"\006"),
		KEY(RIGHT,	"\033OC"),
		KEY(RIGHT,	"\033[C"),
		KEY(UP,		"\020"),
		KEY(UP,		"\033OA"),
		KEY(UP,		"\033[A"),
		{ 0, NULL, 0 },
	};
	int	c, i;

	*nread = 0;
	for (; size > 0; size--) {
		buf[(*nread)++] = tty_getc();

		for (i = 0; keys[i].s != NULL; i++) {
			if (*nread > keys[i].length
			    || strncmp(buf, keys[i].s, *nread) != 0)
				continue;

			if (*nread == keys[i].length)
				return keys[i].key;

			/* Partial match found, continue reading. */
			break;
		}
		if (keys[i].s == NULL)
			break;
	}

	if (*nread > 1 && buf[0] == '\033' && (buf[1] == '[' || buf[1] == 'O')) {
		/*
		 * An escape sequence which is not a supported key is being
		 * read. Discard the rest of the sequence.
		 */
		c = buf[(*nread) - 1];
		while (c < '@' || c > '~')
			c = tty_getc();

		return UNKNOWN;
	}

	if (!isu8start(buf[0])) {
		if (isprint(buf[0]))
			return PRINTABLE;

		return UNKNOWN;
	}

	/*
	 * Ensure a whole Unicode character is read. The number of MSBs in the
	 * first octet of a Unicode character is equal to the number of octets
	 * the character consists of, followed by a zero. Therefore, as long as
	 * the MSB is not zero there is still bytes left to read.
	 */
	for (;;) {
		if ((((unsigned int)buf[0] << *nread) & 0x80) == 0)
			break;
		if (size == 0)
			return UNKNOWN;

		buf[(*nread)++] = tty_getc();
		size--;
	}

	return PRINTABLE;
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
	return MB_CUR_MAX > 1 && (c & (0x80 | 0x40)) == 0x80;
}

int
isu8start(unsigned char c)
{
	return MB_CUR_MAX > 1 && (c & (0x80 | 0x40)) == (0x80 | 0x40);
}
