ifeq ($(OS), Windows_NT)
    USER_OS := Windows
else
    USER_OS := $(shell uname -s)
endif

ifneq ($(USER_OS), Darwin)
	ASAN_OPTIONS := detect_leaks=1
endif

RESULT := backtrace
CFLAGS := -g -fsanitize=address,undefined -Wall -Werror -O0 -no-pie -fno-omit-frame-pointer

all: run

run: compile
	ASAN_OPTIONS=$(ASAN_OPTIONS) ./$(RESULT)

compile: $(wildcard *.h) $(wildcard *.c)
	gcc $(CFLAGS) backtrace.c main.c -o $(RESULT)

clean:
	rm -f $(RESULT)
