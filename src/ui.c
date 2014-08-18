#include <curses.h>
#include <err.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_FULL_QUEUE_H
#include <sys/queue.h>
#else /* HAVE_FULL_QUEUE_H */
#include "compat/queue.h"
#endif /* HAVE_FULL_QUEUE_H */

#include "choice.h"
#include "choices.h"
#include "ui.h"

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

	vis_choices = 0;
	SLIST_FOREACH(c, cs, choices) {
		if (vis_choices == LINES - 1 || c->score == 0)
			break;
		put_line(
		    vis_choices + 1,
		    c->str,
		    strlen(c->str),
		    vis_choices == sel);
		++vis_choices;
	}
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

struct choice *
get_selected(struct choices *cs, char *initial_query)
{
	int ch;
	char *query;
	size_t query_pos;
	size_t query_size;
	size_t query_len;
	size_t initial_query_len;
	int sel;
	int vis_choices;

	initial_query_len = strlen(initial_query);
	query_size = 64;
	if (query_size < initial_query_len + 1)
		query_size = initial_query_len + 1;
	if ((query = calloc(query_size, sizeof(char))) == NULL)
		err(1, "calloc");
	strcpy(query, initial_query);

	query_pos = initial_query_len;
	query_len = initial_query_len;
	sel = 0;

	choices_score(cs, query);
	choices_sort(cs);
	start_curses();
	put_line(0, query, query_len, 0);
	vis_choices = put_choices(cs, sel);
	move(0, query_pos);
	refresh();

	while((ch = getch()) != ERR) {
		switch(ch) {
		case 10: /* Enter */
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
		case KEY_LEFT:
			if (query_pos > 0)
				--query_pos;
			break;
		case KEY_RIGHT:
			if (query_pos < query_len)
				++query_pos;
			break;
		case 127: /* Backspace */
			if (query_pos > 0) {
				memmove(
				    query + query_pos - 1,
				    query + query_pos,
				    query_len - query_pos + 1);
				--query_pos;
				--query_len;
				choices_score(cs, query);
				choices_sort(cs);
				sel = 0;
			}
			break;
		default:
			if (ch > 31 && ch < 127) { /* Printable chars */
				if (query_pos < query_len)
					memmove(
					    query + query_pos + 1,
					    query + query_pos,
					    query_len - query_pos);
				query[query_pos++] = ch;
				query[++query_len] = '\0';
				choices_score(cs, query);
				choices_sort(cs);
				sel = 0;
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
		move(0, query_pos);
		refresh();
	}

	err(1, "getch");
}
