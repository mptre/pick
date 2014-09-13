#include <curses.h>
#include <err.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <bsd/string.h>

#ifdef HAVE_FULL_QUEUE_H
#include <sys/queue.h>
#else /* HAVE_FULL_QUEUE_H */
#include "compat/queue.h"
#endif /* HAVE_FULL_QUEUE_H */

#include "choice.h"
#include "choices.h"
#include "ui.h"

#define KEY_CTRL_A 1
#define KEY_CTRL_B 2
#define KEY_CTRL_D 4
#define KEY_CTRL_E 5
#define KEY_CTRL_F 6
#define KEY_CTRL_K 11
#define KEY_CTRL_U 21
#define KEY_CTRL_W 23
#define KEY_DEL 127
#define KEY_REAL_ENTER 10 /* curses.h defines KEY_ENTER to be Ctrl-Enter */

int stdoutfd;

void
start_curses()
{
	int fd;

	freopen("/dev/tty", "r", stdin);
	setlocale(LC_ALL, "");
	fflush(stdout);
	stdoutfd = dup(STDOUT_FILENO);
	fd = open("/dev/tty", O_WRONLY);
	dup2(fd, STDOUT_FILENO);
	close(fd);
	initscr();
	cbreak();
	noecho();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
}

void
stop_curses()
{

	endwin();
	fflush(stdout);
	dup2(stdoutfd, STDOUT_FILENO);
	close(stdoutfd);
}

void
put_line(int y, char *str, int len, int so)
{
	if (so)
		standout();
	if (len > 0)
		mvaddstr(y, 0, str);
	move(y, len);
	for (; len < COLS; ++len)
		addch(' ');
	if (so)
		standend();
}

int
put_choices(struct choices *cs, int sel)
{
	struct choice *c;
	int vis_choices;
	int i;
	char *line;
	size_t len;
	size_t llen;

	llen = 64;
	if ((line = malloc(sizeof(char) * llen)) == NULL)
		err(1, "malloc");

	vis_choices = 0;
	SLIST_FOREACH(c, cs, choices) {
		len = strlen(c->str) + strlen(c->desc) + 1;
		while (len > llen) {
			llen = llen * 2;
			if ((line = realloc(line, llen)) == NULL) {
				err(1, "realloc");
			}
		}
		strlcpy(line, c->str, llen);
		strlcat(line, " ", llen);
		strlcat(line, c->desc, llen);

		if (vis_choices == LINES - 1 || c->score == 0)
			break;
		put_line(
		    vis_choices + 1,
		    line,
		    len,
		    vis_choices == sel);
		++vis_choices;
	}
	free(line);
	for (i = vis_choices + 1; i < LINES; ++i)
		put_line(i, "", 0, 0);

	return vis_choices;
}

struct choice *
selected(struct choices *cs, int sel)
{
	struct choice *c;
	int i;

	i = 0;
	SLIST_FOREACH(c, cs, choices) {
		if (c->score == 0)
			break;
		if (i == sel)
			return c;
		++i;
	}
	return NULL;
}

void
filter_choices(struct choices *cs, char *query, int *sel)
{
	choices_score(cs, query);
	choices_sort(cs);
	*sel = 0;
}

void
del_chars_between(char *str, size_t len, size_t start, size_t end)
{
	memmove(
	    str + start,
	    str + end,
	    len - end + 1);
}

struct choice *
get_selected(struct choices *cs, char *initial_query)
{
	int ch;
	char *query;
	size_t cursor_pos;
	size_t query_size;
	size_t query_len;
	size_t initial_query_len;
	int sel;
	int vis_choices;
	int word_pos;

	initial_query_len = strlen(initial_query);
	cursor_pos = initial_query_len;
	query_len = initial_query_len;
	query_size = 64;
	if (query_size < initial_query_len + 1)
		query_size = initial_query_len + 1;
	if ((query = calloc(query_size, sizeof(char))) == NULL)
		err(1, "calloc");
	strcpy(query, initial_query);

	filter_choices(cs, query, &sel);
	start_curses();

	put_line(0, query, query_len, 0);
	vis_choices = put_choices(cs, sel);
	move(0, cursor_pos);
	refresh();

	while((ch = getch()) != ERR) {
		switch(ch) {
		case KEY_REAL_ENTER:
			if (vis_choices > 0) {
				stop_curses();
				free(query);
				return selected(cs, sel);
			}
		case KEY_DOWN:
			if (sel < vis_choices - 1)
				++sel;
			break;
		case KEY_UP:
			if (sel > 0)
				--sel;
			break;
		case KEY_CTRL_B:
		case KEY_LEFT:
			if (cursor_pos > 0)
				--cursor_pos;
			break;
		case KEY_CTRL_F:
		case KEY_RIGHT:
			if (cursor_pos < query_len)
				++cursor_pos;
			break;
		case KEY_BACKSPACE:
		case KEY_DEL:
			if (cursor_pos > 0) {
				del_chars_between(
				    query,
				    query_len,
				    cursor_pos - 1,
				    cursor_pos);
				--cursor_pos;
				--query_len;
				filter_choices(cs, query, &sel);
			}
			break;
		case KEY_CTRL_D:
			if (cursor_pos < query_len) {
				del_chars_between(
				    query,
				    query_len,
				    cursor_pos,
				    cursor_pos + 1);
				--query_len;
				filter_choices(cs, query, &sel);
			}
			break;
		case KEY_CTRL_U:
			del_chars_between(
			    query,
			    query_len,
			    0,
			    cursor_pos);
			query_len -= cursor_pos;
			cursor_pos = 0;
			filter_choices(cs, query, &sel);
			break;
		case KEY_CTRL_K:
			del_chars_between(
			    query,
			    query_len,
			    cursor_pos + 1,
			    query_len);
			query_len = cursor_pos;
			filter_choices(cs, query, &sel);
			break;
		case KEY_CTRL_W:
			if (cursor_pos > 0) {
				for (word_pos = cursor_pos - 1;
				    word_pos > 0;
				    --word_pos)
					if (query[word_pos] != ' ' &&
					    query[word_pos - 1] == ' ')
						break;
				del_chars_between(
				    query,
				    query_len,
				    word_pos,
				    cursor_pos);
				query_len -= cursor_pos - word_pos;
				cursor_pos = word_pos;
				filter_choices(cs, query, &sel);
			}
			break;
		case KEY_CTRL_A:
			cursor_pos = 0;
			break;
		case KEY_CTRL_E:
			cursor_pos = query_len;
			break;
		default:
			if (ch > 31 && ch < 127) { /* Printable chars */
				if (cursor_pos < query_len)
					memmove(
					    query + cursor_pos + 1,
					    query + cursor_pos,
					    query_len - cursor_pos);
				query[cursor_pos++] = ch;
				query[++query_len] = '\0';
				filter_choices(cs, query, &sel);
			}
			break;
		}
		if (query_len == query_size - 1) {
			query_size += query_size;
			if ((query = realloc(
					    query,
					    query_size * sizeof(char))) == NULL)
				err(1, "realloc");
		}
		put_line(0, query, query_len, 0);
		vis_choices = put_choices(cs, sel);
		move(0, cursor_pos);
		refresh();
	}

	err(1, "getch");
}
