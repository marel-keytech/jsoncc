CC := gcc
AR := ar
CFLAGS := -Wall -fvisibility=hidden -std=c99 -D_GNU_SOURCE -O3 -Isrc/
LDFLAGS :=

MAJOR = 0
MINOR = 0
PATCH = 1

DYNAMIC_LIB = libjsonparsergen.so.$(MAJOR).$(MINOR).$(PATCH)
STATIC_LIB = libjsonparsergen.a
BINARY = jsonparsergen

PREFIX ?= /usr/local

LIBDIR = $(DESTDIR)$(PREFIX)/lib
BINDIR = $(DESTDIR)$(PREFIX)/bin
INCLUDE = $(DESTDIR)$(PREFIX)/include

all: src/parser.c src/json_parser.c $(BINARY) $(DYNAMIC_LIB) $(STATIC_LIB)

$(BINARY): src/main.o src/lexer.o src/parser.o src/obj.o src/codegen.o
	$(CC) $(LDFLAGS) $^ -o $@

$(DYNAMIC_LIB): src/json_lexer.o src/json_parser.o src/json_obj.o
	$(CC) -shared $(LDFLAGS) $^ -o $@

$(STATIC_LIB): src/json_lexer.o src/json_parser.o src/json_obj.o
	$(AR) rcs $@ $^

.PHONY: .c.o
.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(BINARY) $(DYNAMIC_LIB) $(STATIC_LIB)
	rm -f src/*.o
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

install: $(BINARY) $(DYNAMIC_LIB) $(STATIC_LIB)
	install $(BINARY) $(BINDIR)
	install $(DYNAMIC_LIB) $(LIBDIR)
	install $(STATIC_LIB) $(LIBDIR)
	cp src/json_obj.h $(INCLUDE)/jsonparsergen.h

# vi: noet sw=8 ts=8 tw=80

