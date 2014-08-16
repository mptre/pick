%.o: %.c
	gcc -g -Wall -Wextra -pedantic-errors -Wno-unused-parameter -Werror -c $<

pick: main.o choice.o io.o ui.o
	gcc $^ -lform -lmenu -lncurses -o $@

clean:
	rm -f *.o pick
