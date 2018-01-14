EXEC = hw4_mm_test
TARGETS = $(EXEC)
CC ?= gcc
CFLAGS += -std=gnu99 -I./lib -Wall
OBJS = hw4_mm_test.o ./lib/hw_malloc.o
SUBDIR = ./lib

all: $(TARGETS)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

hw4_mm_test.o: %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

./lib/hw_malloc.o: ./lib/hw_malloc.c ./lib/hw_malloc.h
	$(MAKE) -C $(SUBDIR)

clean:
	rm -rf *.o $(EXEC)
	$(MAKE) -C $(SUBDIR) clean

astyle:
	astyle --style=linux --indent=tab --max-code-length=80 --suffix=none *.c *.h lib/*.c lib/*.h
