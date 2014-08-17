#include <bsd/sys/queue.h>
#include <curses.h>
#include <err.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
get_selected(struct choices *cs)
{
	int ch;
	char *query;
	size_t pos;
	size_t size;
	size_t len;
	int sel;
	int vis_choices;

	size = 64;
	if ((query = calloc(size, sizeof(char))) == NULL)
		err(1, "calloc");

	pos = 0;
	len = 0;
	sel = 0;

	start_curses();
	vis_choices = put_choices(cs, sel);
	move(0, pos);
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
			if (pos > 0)
				--pos;
			break;
		case KEY_RIGHT:
			if (pos < len)
				++pos;
			break;
		case 127: /* Backspace */
			if (pos > 0) {
				memmove(query + pos - 1, query + pos, len - pos + 1);
				--pos;
				--len;
				choices_score(cs, query);
				choices_sort(cs);
				sel = 0;
			}
			break;
		default:
			if (ch > 31 && ch < 127) { /* Printable chars */
				if (pos < len)
					memmove(query + pos + 1, query + pos, len - pos);
				query[pos++] = ch;
				query[++len] = '\0';
				choices_score(cs, query);
				choices_sort(cs);
				sel = 0;
			}
			break;
		}
		if (len == size - 1) {
			size += size;
			if ((query = realloc(query, size * sizeof(char))) == NULL)
				err(1, "realloc");
		}
		put_line(0, query, len, 0);
		vis_choices = put_choices(cs, sel);
		move(0, pos);
		refresh();
	}

	err(1, "getch");
}
