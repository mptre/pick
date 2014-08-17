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
#include "ui.h"

void
init_ui()
{
    freopen("/dev/tty", "r", stdin);
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    refresh();
}

void
free_ui()
{
    endwin();
}

void
mvaddln(int y, char *str, int len, int so)
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
draw_choices(struct choices *cs, int sel)
{
	struct choice *c;
	int nchoices;
	int i;

	nchoices = 0;
	SLIST_FOREACH(c, cs, choices) {
		if (nchoices + 1 == LINES)
			break;
		if (c->score > 0) {
			mvaddln(nchoices + 1, c->str, strlen(c->str), nchoices == sel);
			++nchoices;
		}
	}
	for (i = nchoices; i < LINES; ++i)
		mvaddln(i + 1, "", 0, 0);

	return nchoices;
}

struct choice *
sel_choice(struct choices *cs, int sel)
{
	struct choice *c;
	int i;

	i = 0;
	SLIST_FOREACH(c, cs, choices) {
		if (c->score > 0) {
			if (i == sel) {
				return c;
			}
			++i;
		}
	}
	return NULL;
}

char *
run_ui(struct choices *cs)
{
	int ch;
	char *q;
	size_t pos;
	size_t size;
	size_t len;
	int sel;
	int nchoices;

	size = 64;
	if ((q = calloc(size, sizeof(char))) == NULL)
		err(1, "calloc");

	pos = 0;
	len = 0;
	sel = 0;

	init_ui();
	nchoices = draw_choices(cs, sel);
	move(0, pos);
	refresh();

	while((ch = getch()) != ERR) {
		switch(ch) {
		case 10: /* Enter */
			if (nchoices > 0) {
				free_ui();
				free(q);
				return sel_choice(cs, sel)->str;
			}
		case KEY_DOWN:
			if (sel < nchoices - 1)
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
				memmove(q + pos - 1, q + pos, len - pos + 1);
				--pos;
				--len;
				choices_score(cs, q);
				choices_sort(cs);
				sel = 0;
			}
			break;
		default:
			if (ch > 31 && ch < 127) { /* Printable chars */
				if (pos < len)
					memmove(q + pos + 1, q + pos, len - pos);
				q[pos++] = ch;
				q[++len] = '\0';
				choices_score(cs, q);
				choices_sort(cs);
				sel = 0;
			}
			break;
		}
		if (len == size - 1) {
			size += size;
			if ((q = realloc(q, size * sizeof(char))) == NULL)
				err(1, "realloc");
	    	}
		mvaddln(0, q, len, 0);
		nchoices = draw_choices(cs, sel);
		move(0, pos);
		refresh();
	}

	err(1, "getch");
}
