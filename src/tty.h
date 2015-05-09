#ifndef TTY_H
#define TTY_H

void tty_init(int);
void tty_restore();
int tty_getc();
void tty_putc(int);
void tty_show_cursor();
void tty_hide_cursor();
void tty_enter_standout_mode();
void tty_exit_standout_mode();
void tty_move_cursor_to(int, int);

#endif /* TTY_H */
