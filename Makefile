export MODULE := Lab1

CC = gcc
CFLAGS += -O1 -std=gnu11 -ggdb -Wall -Werror -Wno-unused-result -Wno-unused-value -Wno-unused-variable
INCLUDES = -Iinclude
SRCS = src/main.c src/expr.c
OBJS = main.o expr.o
TARGET = gen-expr

.PHONY: all git test clean commit-and-make

.DEFAULT_GOAL := commit-and-make
commit-and-make: git all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

main.o: src/main.c include/expr.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

expr.o: src/expr.c include/expr.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f *.o $(TARGET)

include Makefile.lab