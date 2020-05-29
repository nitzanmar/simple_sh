CFLAGS=-Wall -Wextra -Weffc++ -Wsign-conversion -pedantic-errors -g3

all: main

main: main.o
	$(CC) -o simple_sh main.o

main.o: main.c
	$(CC) -c $(CFLAGS) main.c

clean:
	rm -rf simple_sh main.o
