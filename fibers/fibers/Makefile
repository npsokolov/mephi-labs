all: run

RESULT=main
CFLAGS=-g -fsanitize=address -fPIC -Wall -Werror

compile: $(wildcard .h) $(wildcard *.c) $(wildcard *.S)
	gcc main.c fiber.c fiber_asm.S -o $(RESULT) $(CFLAGS)

run: compile
	./$(RESULT)

clean:
	rm -f $(RESULT)
