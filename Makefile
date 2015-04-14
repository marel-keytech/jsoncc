CC := gcc
AR := ar
CFLAGS = -Wall -fvisibility=hidden -std=c99 -D_GNU_SOURCE -O0 -g -Isrc/\
       	-I/usr/include/lua5.1\
       	-DTEMPLATE_PATH='"$(TEMPLATE_PATH)"'
LDFLAGS := -llua5.1

MAJOR = 0
MINOR = 0
PATCH = 1

DYNAMIC_LIB = libjsonparsergen.so.$(MAJOR).$(MINOR).$(PATCH)
STATIC_LIB = libjsonparsergen.a
BINARY = jsonparsergen

PREFIX ?= /usr/local


LIBDIR=$(DESTDIR)$(PREFIX)/lib
BINDIR=$(DESTDIR)$(PREFIX)/bin
INCLUDE=$(DESTDIR)$(PREFIX)/include
SHAREDIR=$(DESTDIR)$(PREFIX)/share

TEMPLATE_PATH = $(SHAREDIR)/jsonparsergen/templates

all: src/parser.c src/json_parser.c $(BINARY) $(DYNAMIC_LIB) $(STATIC_LIB)

$(BINARY): src/main.o src/jslex.o src/desc_parser.o src/obj.o
	$(CC) $^ $(LDFLAGS) -o $@

$(DYNAMIC_LIB): src/json_lexer.o src/json_parser.o src/json_obj.o src/json_string.o
	$(CC) -shared $(LDFLAGS) $^ -o $@

$(STATIC_LIB): src/json_lexer.o src/json_parser.o src/json_obj.o src/json_string.o
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

src/json_lexer.c: src/json_lexer.rl
	ragel -C -G2 $^ -o $@

tst/json_string_test: src/json_string.c tst/json_string_test.c
	$(CC) -O0 -g -Isrc/ $^ -o $@

.PHONY:
test: tst/json_string_test
	run-parts -v tst

install: $(BINARY) $(DYNAMIC_LIB) $(STATIC_LIB)
	install $(BINARY) $(BINDIR)
	install $(DYNAMIC_LIB) $(LIBDIR)
	install $(STATIC_LIB) $(LIBDIR)
	mkdir -p $(TEMPLATE_PATH)
	install templates/*.lua $(TEMPLATE_PATH)
	cp src/json_obj.h $(INCLUDE)/jsonparsergen.h

# vi: noet sw=8 ts=8 tw=80

