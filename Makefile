export MODULE := Lab1

CC = gcc
CFLAGS += -O1 -std=gnu11 -ggdb -Wall -Werror -Wno-unused-result -Wno-unused-value -Wno-unused-variable
INCLUDES = -Iinclude
SRCS := $(wildcard src/*.c)
DEPS := $(wildcard include/*.h) $(SRCS)
OBJS := $(SRCS:src/%.c=%.o)
TARGET = gen-expr

.PHONY: all git test clean commit-and-make

.DEFAULT_GOAL := commit-and-make
commit-and-make: git all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

%.o: src/%.c $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f *.o $(TARGET)

include Makefile.lab