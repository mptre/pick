#include <bsd/sys/queue.h>
#include <curses.h>
#include <err.h>
#include <fcntl.h>
#include <form.h>
#include <locale.h>
#include <menu.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "choice.h"
#include "ui.h"

WINDOW *window;
FIELD *fields[2];
FORM *form;
MENU *menu;
ITEM **items;
size_t maxitems;

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
    fields[0] = new_field(1, COLS-1, 0, 0, 0, 0);
    fields[1] = NULL;
    field_opts_off(fields[0], O_STATIC);
    field_opts_off(fields[0], O_BLANK);
    form = new_form(fields);
    post_form(form);
    maxitems = 64;
    items = calloc(maxitems, sizeof(ITEM *));
    items[0] = NULL;
    window = newwin(LINES - 1, COLS, 1, 0);
    keypad(window, TRUE);
    menu = new_menu(items);
    set_menu_win(menu, window);
    set_menu_mark(menu, "");
    post_menu(menu);
    refresh();
    wrefresh(window);
    refresh();
}

void
free_ui()
{
    int i;

    unpost_form(form);
    free_form(form);
    free_field(fields[0]);
    unpost_menu(menu);
    free_menu(menu);
    for (i = 0; items[i] != NULL; ++i) {
        free_item(items[i]);
    }
    endwin();
}

char *
run_ui(struct choices *cs)
{
	struct choice *c;
	char *query;
	char *sel;
	int ch;
	size_t i;

	init_ui();
	i = 0;
	SLIST_FOREACH(c, cs, choices) {
		if (i == maxitems - 1) {
			maxitems = 2 * maxitems;
			if ((items = realloc(items, maxitems * sizeof(ITEM *))) == NULL)
				err(1, "realloc");
		}
		items[i] = new_item(c->str, "");
		++i;
	}
	items[i] = NULL;
	unpost_menu(menu);
	free_menu(menu);
	menu = new_menu(items);
	set_menu_win(menu, window);
	set_menu_mark(menu, "");
	post_menu(menu);
	refresh();
	wrefresh(window);
	refresh();

	while((ch = getch()) != ERR) {
		switch(ch) {
		case KEY_DOWN:
			curs_set(0);
			menu_driver(menu, REQ_DOWN_ITEM);
			break;
		case KEY_UP:
			curs_set(0);
			menu_driver(menu, REQ_UP_ITEM);
			break;
		case KEY_LEFT:
			curs_set(1);
			form_driver(form, REQ_PREV_CHAR);
			break;
		case KEY_RIGHT:
			curs_set(1);
			form_driver(form, REQ_RIGHT_CHAR);
			break;
		case 127: /* Backspace */
			curs_set(1);
			form_driver(form, REQ_DEL_PREV);
			break;
		case 10: /* Enter */
			curs_set(0);
			sel = strdup(item_name(current_item(menu)));
			free_ui();
			return sel;
		default:
			curs_set(1);
			form_driver(form, ch);
			if (form_driver(form, REQ_VALIDATION) != E_OK)
				err(1, "validation");
			query = strdup(field_buffer(fields[0], 0));
			free(query);
			break;
		}
		wrefresh(window);
	}

	err(1, "getch");
}
