CC := gcc
CFLAGS := -Wall -g -std=c99 -D_GNU_SOURCE -O0 -Isrc/
LDFLAGS := -lrt

BINARY = jsonunpackgen

PREFIX ?= /usr/local

BINDIR = $(DESTDIR)$(PREFIX)/bin

all: src/parser.c $(BINARY)

$(BINARY): src/main.o src/lexer.o src/parser.o src/obj.o src/codegen.o
	$(CC) $(LDFLAGS) $^ -o $@

.PHONY: .c.o
.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(BINARY)
	rm -f src/*.o
	rm -f tst/test_*
	rm -f src/parser.c
	rm -f src/parser.h
	rm -f src/parser.out
	rm -f src/lexer.c

src/parser.c: src/parser.y
	lemon $^

src/lexer.c: src/lexer.rl
	ragel -C -G2 $^ -o $@

.PHONY:
test: 
	run-parts -v tst

install:
	install $(BINARY) $(BINDIR)

# vi: noet sw=8 ts=8 tw=80

