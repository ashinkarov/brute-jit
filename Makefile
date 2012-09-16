

AR :=  ar rc
CFLAGS := -Wall -Wextra -pedantic -std=c99 -O3 -D_GNU_SOURCE -D_BSD_SOURCE


all: example


libdynamic.a: shared-library.o compile.o
	$(AR) $@ $^

compile.o: compile.h

shared-library.o: shared-library.h
	$(CC) $(CFLAGS) -c -ldl -o$@ shared-library.c

example: libdynamic.a example.c compile.h shared-library.h
	$(CC) $(CFLAGS) -o$@ example.c -L. -ldl -ldynamic


clean:
	$(RM) *.o example libdynamic.a
