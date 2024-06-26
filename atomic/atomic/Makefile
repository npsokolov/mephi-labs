all: run

RESULT=main
CFLAGS=-g -O2 -Wall -Werror -lpthread

compile: $(wildcard .h) $(wildcard *.c)
	! grep pthread atomic.h
	gcc main.c -o $(RESULT) $(CFLAGS)

run: compile
	./$(RESULT)

clean:
	rm -f $(RESULT)
