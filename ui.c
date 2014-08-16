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

char *
run_ui(struct choices *cs)
{
	struct choice *c;
	int ch;
	char *q;
	size_t pos;
	size_t size;
	size_t len;
	int sel;
	int i;

	size = 3;
	if ((q = calloc(size, sizeof(char))) == NULL)
		err(1, "calloc");

	pos = 0;
	len = 0;
	sel = 0;

	init_ui();
	i = 0;
	SLIST_FOREACH(c, cs, choices) {
		if (i + 1 == LINES)
			break;
		if (c->score > 0) {
			if (i == sel)
				standout();
			mvaddstr(i + 1, 0, c->str);
			if (i == sel)
				standend();
			++i;
		}
	}
	move(0, pos);
	refresh();

	while((ch = getch()) != ERR) {
		switch(ch) {
		case 10: /* Enter */
			free_ui();
			free(q);
			return "picked";
		case KEY_DOWN:
			if (sel < LINES - 2)
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
		case 127:
			if (pos > 0) {
				memmove(q + pos - 1, q + pos, len - pos + 1);
				--pos;
				--len;
				choices_score(cs, q);
				choices_sort(cs);
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
			}
			break;
		}
		if (len == size - 1) {
			size += size;
			if ((q = realloc(q, size * sizeof(char))) == NULL)
				err(1, "realloc");
	    	}
		if (len > 0)
			mvaddstr(0, 0, q);
		move(0, len);
		for (i = len; i < COLS; ++i)
			addch(' ');
		i = 0;
		SLIST_FOREACH(c, cs, choices) {
			if (i + 1 == LINES)
				break;
			if (c->score > 0) {
				if (i == sel)
					standout();
				mvaddstr(i + 1, 0, c->str);
				if (i == sel)
					standend();
				++i;
			}
		}
		for (; i < LINES; ++i)
			mvaddstr(i + 1, 0, "                               ");

		move(0, pos);
		refresh();
	}

	err(1, "getch");
}
