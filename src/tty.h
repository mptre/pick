#ifndef TTY_H
#define TTY_H

void tty_init(int);
void tty_restore();
int tty_getc();
int tty_putc(int);
void tty_putp(const char *);

#endif /* TTY_H */
