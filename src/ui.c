#include "config.h"
#include <err.h>
#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_NCURSESW_H
#include <ncursesw/curses.h>
#include <ncursesw/term.h>
#else
#include <curses.h>
#include <term.h>
#endif

#ifdef HAVE_FULL_QUEUE_H
#include <sys/queue.h>
#else /* HAVE_FULL_QUEUE_H */
#include "compat/queue.h"
#endif /* HAVE_FULL_QUEUE_H */

#ifndef HAVE_STRLCPY
#include "compat/strlcpy.h"
#endif /* !HAVE_STRLCPY */

#ifndef HAVE_STRLCAT
#include "compat/strlcat.h"
#endif /* !HAVE_STRLCAT */

#include "choice.h"
#include "choices.h"
#include "ui.h"
#include "tty.h"

#define KEY_CTRL_A 1
#define KEY_CTRL_B 2
#define KEY_CTRL_D 4
#define KEY_CTRL_E 5
#define KEY_CTRL_F 6
#define KEY_CTRL_N 14
#define KEY_CTRL_P 16
#define KEY_CTRL_K 11
#define KEY_CTRL_U 21
#define KEY_CTRL_W 23
#define KEY_DEL 127
#define KEY_REAL_ENTER 10 /* curses.h defines KEY_ENTER to be Ctrl-Enter */
#define KEY_ESCAPE 27
#define KEY_BRACKET 91
#define KEY_RAW_O 79
#define KEY_RAW_DOWN 66
#define KEY_RAW_UP 65
#define KEY_RAW_LEFT 68
#define KEY_RAW_RIGHT 67

static void print_line(int, char *, int, int);
static int print_choices(struct choices *, int);
static struct choice *selected_choice(struct choices *, int);
static void filter_choices(struct choices *, char *, int *);
static void delete_between(char *, size_t, size_t, size_t);
static void print_string_at(int, int, char *);
static void move_cursor_to(int, int);

struct choice *
ui_selected_choice(struct choices *choices, char *initial_query,
    int use_alternate_screen)
{
	char *query;
	int key, selection, visible_choices_count, word_position;
	size_t cursor_position, query_size, query_length, initial_query_length;

	initial_query_length = strlen(initial_query);
	cursor_position = initial_query_length;
	query_length = initial_query_length;
	query_size = 64;

	if (query_size < initial_query_length + 1) {
		query_size = initial_query_length + 1;
	}

	query = calloc(query_size, sizeof(char));
	if (query == NULL) {
		err(1, "calloc");
	}

	strlcpy(query, initial_query, query_size);

	filter_choices(choices, query, &selection);
	tty_init(use_alternate_screen);

	print_line(0, query, query_length, 0);
	visible_choices_count = print_choices(choices, selection);
	move_cursor_to(0, cursor_position);
	tty_putp(cursor_normal);

	while((key = tty_getc()) != ERR) {
		switch(key) {
		case KEY_REAL_ENTER:
			if (visible_choices_count > 0) {
				tty_restore();
				free(query);
				return selected_choice(choices, selection);
			}
		case KEY_CTRL_N:
			if (selection < visible_choices_count - 1) {
				++selection;
			}

			break;
		case KEY_CTRL_P:
			if (selection > 0) {
				--selection;
			}

			break;
		case KEY_CTRL_B:
			if (cursor_position > 0) {
				--cursor_position;
			}

			break;
		case KEY_CTRL_F:
			if (cursor_position < query_length) {
				++cursor_position;
			}

			break;
		case KEY_BACKSPACE:
		case KEY_DEL:
			if (cursor_position > 0) {
				delete_between(
				    query,
				    query_length,
				    cursor_position - 1,
				    cursor_position);
				--cursor_position;
				--query_length;
				filter_choices(choices, query, &selection);
			}

			break;
		case KEY_CTRL_D:
			if (cursor_position < query_length) {
				delete_between(
				    query,
				    query_length,
				    cursor_position,
				    cursor_position + 1);
				--query_length;
				filter_choices(choices, query, &selection);
			}

			break;
		case KEY_CTRL_U:
			delete_between(
			    query,
			    query_length,
			    0,
			    cursor_position);
			query_length -= cursor_position;
			cursor_position = 0;
			filter_choices(choices, query, &selection);
			break;
		case KEY_CTRL_K:
			delete_between(
			    query,
			    query_length,
			    cursor_position + 1,
			    query_length);
			query_length = cursor_position;
			filter_choices(choices, query, &selection);
			break;
		case KEY_CTRL_W:
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
				filter_choices(choices, query, &selection);
			}
			break;
		case KEY_CTRL_A:
			cursor_position = 0;
			break;
		case KEY_CTRL_E:
			cursor_position = query_length;
			break;
		case KEY_ESCAPE:
			if((key = tty_getc()) != ERR) {
				if (key == KEY_BRACKET || key == KEY_RAW_O) {
					if((key = tty_getc()) != ERR) {
						switch (key) {
						case KEY_RAW_DOWN:
							if (selection < visible_choices_count - 1) {
								++selection;
							}

							break;
						case KEY_RAW_UP:
							if (selection > 0) {
								--selection;
							}

							break;
						case KEY_RAW_LEFT:
							if (cursor_position > 0) {
								--cursor_position;
							}

							break;
						case KEY_RAW_RIGHT:
							if (cursor_position <
							    query_length) {
								++cursor_position;
							}

							break;
						}
					} else {
						err(1, "tty_getc");
					}
				}
			} else {
				err(1, "tty_getc");
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
				filter_choices(choices, query, &selection);
			}

			break;
		}

		tty_putp(cursor_invisible);

		if (query_length == query_size - 1) {
			query_size += query_size;

			query = realloc(query, query_size * sizeof(char));
			if (query == NULL) {
				err(1, "realloc");
			}
		}

		print_line(0, query, query_length, 0);
		visible_choices_count = print_choices(choices, selection);
		move_cursor_to(0, cursor_position);
		tty_putp(cursor_normal);
	}

	err(1, "tty_getc");
}

static void
print_line(int y, char *string, int length, int standout)
{
	if (standout) {
		tty_putp(enter_standout_mode);
	}

	if (length > 0) {
		print_string_at(y, 0, string);
	}

	move_cursor_to(y, length);

	for (; length < COLS; ++length) {
		tty_putc(' ');
	}

	if (standout) {
		tty_putp(exit_standout_mode);
	}
}

static int
print_choices(struct choices *choices, int selection)
{
	char *line;
	int i;
	int visible_choices_count = 0;
	size_t length;
	size_t line_length = 64;
	struct choice *choice;

	line = calloc(sizeof(char), line_length);
	if (line == NULL) {
		err(1, "calloc");
	}

	SLIST_FOREACH(choice, choices, choices) {
		length = strlen(choice->string) + strlen(choice->description) +
		    1;

		while (length > line_length) {
			line_length = line_length * 2;

			line = realloc(line, line_length);
			if (line == NULL) {
				err(1, "realloc");
			}
		}

		strlcpy(line, choice->string, line_length);
		strlcat(line, " ", line_length);
		strlcat(line, choice->description, line_length);

		if (visible_choices_count == LINES - 1 || choice->score == 0) {
			break;
		}

		print_line(visible_choices_count + 1, line, length,
		    visible_choices_count == selection);

		++visible_choices_count;
	}

	free(line);

	for (i = visible_choices_count + 1; i < LINES; ++i) {
		print_line(i, "", 0, 0);
	}

	return visible_choices_count;
}

static struct choice *
selected_choice(struct choices *choices, int selection)
{
	struct choice *choice;
	int i = 0;

	SLIST_FOREACH(choice, choices, choices) {
		if (choice->score == 0) {
			break;
		}

		if (i == selection) {
			return choice;
		}

		++i;
	}

	return NULL;
}

static void
filter_choices(struct choices *choices, char *query, int *selection)
{
	choices_score(choices, query);
	choices_sort(choices);
	*selection = 0;
}

static void
delete_between(char *string, size_t length, size_t start, size_t end)
{
	memmove(string + start, string + end, length - end + 1);
}

static void
print_string_at(int y, int x, char *string)
{
	int i;

	move_cursor_to(y, x);

	for (i = 0; string[i] != '\0'; i++) {
		tty_putc(string[i]);
	}
}

static void
move_cursor_to(int y, int x)
{
	tty_putp(tgoto(cursor_address, x, y));
}
