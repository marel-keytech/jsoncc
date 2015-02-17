CC := gcc
CFLAGS := -Wall -g -std=c99 -D_GNU_SOURCE -O0 -Isrc/
LDFLAGS := -lrt

BINARY = jsonunpackgen

PREFIX ?= /usr/local

BINDIR = $(DESTDIR)$(PREFIX)/bin

all: src/parser.c $(BINARY)

$(BINARY): src/main.o src/lexer.o src/parser.o src/obj.o src/codegen.o
	$(CC) $(LDFLAGS) $^ -o $@

jsontest: src/json_test.o src/json_lexer.o src/json_parser.o src/json_obj.o
	$(CC) $(LDFLAGS) $^ -o $@

.PHONY: .c.o
.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(BINARY) jsontest
	rm -f src/*.o
	rm -f tst/test_*
	rm -f src/parser.c src/parser.h src/parser.out src/lexer.c
	rm -f src/json_parser.c src/json_parser.h src/json_parser.out
	rm -f src/json_lexer.c

src/parser.c: src/parser.y
	lemon $^

src/json_parser.c: src/json_parser.y
	lemon $^

src/lexer.c: src/lexer.rl
	ragel -C -G2 $^ -o $@

src/json_lexer.c: src/json_lexer.rl
	ragel -C -G2 $^ -o $@

.PHONY:
test: 
	run-parts -v tst

install:
	install $(BINARY) $(BINDIR)

# vi: noet sw=8 ts=8 tw=80

