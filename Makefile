CC := gcc
AR := ar
CFLAGS = -Wall -fvisibility=hidden -std=c99 -D_GNU_SOURCE -O0 -g -Isrc/\
       	-I/usr/include/lua5.1 \
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

all: $(BINARY) $(DYNAMIC_LIB) $(STATIC_LIB)

$(BINARY): src/main.o src/jslex.o src/desc_parser.o src/obj.o src/lua_obj.o \
	src/lua_codegen.o
	$(CC) $^ $(LDFLAGS) -o $@

$(DYNAMIC_LIB): src/jslex.o src/json_string.o
	$(CC) -shared $(LDFLAGS) $^ -o $@

$(STATIC_LIB): src/jslex.o src/json_string.o
	$(AR) rcs $@ $^

.PHONY: .c.o
.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(BINARY) $(DYNAMIC_LIB) $(STATIC_LIB)
	rm -f src/*.o

tst/json_string_test: src/json_string.c tst/json_string_test.c
	$(CC) -O0 -g -Isrc/ $^ -o $@

.PHONY:
test: tst/json_string_test
	run-parts -v tst

install: $(BINARY) $(DYNAMIC_LIB) $(STATIC_LIB)
	install $(BINARY) $(BINDIR)
	install $(DYNAMIC_LIB) $(LIBDIR)
	install $(STATIC_LIB) $(LIBDIR)
	install src/jslex.h $(INCLUDE)
	mkdir -p $(TEMPLATE_PATH)
	install templates/*.lua $(TEMPLATE_PATH)

# vi: noet sw=8 ts=8 tw=80

