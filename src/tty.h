#ifndef TTY_H
#define TTY_H

#define TTY_CTRL_A 1
#define TTY_CTRL_B 2
#define TTY_CTRL_D 4
#define TTY_CTRL_E 5
#define TTY_CTRL_F 6
#define TTY_CTRL_N 14
#define TTY_CTRL_P 16
#define TTY_CTRL_K 11
#define TTY_CTRL_U 21
#define TTY_CTRL_W 23
#define TTY_DEL 127
#define TTY_ENTER 10
#define TTY_BACKSPACE 263
#define TTY_UP 259
#define TTY_DOWN 258
#define TTY_RIGHT 261
#define TTY_LEFT 260

void tty_init(int);
void tty_restore();
int tty_getch();
void tty_putc(int);
void tty_show_cursor();
void tty_hide_cursor();
void tty_enter_standout_mode();
void tty_exit_standout_mode();
void tty_move_cursor_to(int, int);
int tty_lines();
int tty_columns();

#endif /* TTY_H */
