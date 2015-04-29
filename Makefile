CFLAGS += -Wall -fvisibility=hidden -std=c99 -D_GNU_SOURCE -O3 -fPIC -Isrc/ \
       	-I/usr/include/lua5.1 \
       	-DTEMPLATE_PATH='"$(TEMPLATE_PATH)"'
LDFLAGS += -llua5.1 -lm -ldl

MAJOR = 0
MINOR = 0
PATCH = 1

DYNAMIC_LIB = libjsoncc.so.$(MAJOR).$(MINOR).$(PATCH)
STATIC_LIB = libjsoncc.a
BINARY = jsoncc

PREFIX ?= /usr/local


LIBDIR=$(DESTDIR)$(PREFIX)/lib
BINDIR=$(DESTDIR)$(PREFIX)/bin
INCLUDE=$(DESTDIR)$(PREFIX)/include
SHAREDIR=$(DESTDIR)$(PREFIX)/share

TEMPLATE_PATH = $(SHAREDIR)/jsoncc/templates

all: $(BINARY) $(DYNAMIC_LIB) $(STATIC_LIB)

$(BINARY): src/main.o src/jslex.o src/desc_parser.o src/obj.o src/lua_obj.o \
	src/lua_codegen.o
	$(CC) $^ $(LDFLAGS) -o $@

$(DYNAMIC_LIB): src/jslex.o src/json_string.o
	$(CC) -shared $^ -o $@

$(STATIC_LIB): src/jslex.o src/json_string.o
	$(AR) rcs $@ $^

.PHONY: .c.o
.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(BINARY) $(DYNAMIC_LIB) $(STATIC_LIB)
	rm -f src/*.o
	rm -f tst/*.o
	rm -f tst/test.[ch]

tst/json_string_test: src/json_string.c tst/json_string_test.c
	$(CC) -Wall -O0 -g -Isrc/ $^ -o $@

tst/generator_test: tst/generator_test.o tst/test.o $(STATIC_LIB) 
	$(CC) -Wall -O0 -g -Isrc/ -Itst/ $^ -o $@

tst/generator_test.o: tst/test.h

tst/test.o: tst/test.h tst/test.c

tst/test.c: $(BINARY) tst/test.x
	./$(BINARY) --template-path=templates --source tst/test.x >tst/test.c

tst/test.h: $(BINARY) tst/test.x
	./$(BINARY) --template-path=templates --header tst/test.x >tst/test.h

.PHONY:
test: tst/json_string_test tst/generator_test
	run-parts -v tst

install: $(BINARY) $(DYNAMIC_LIB) $(STATIC_LIB)
	install $(BINARY) $(BINDIR)
	install $(DYNAMIC_LIB) $(LIBDIR)
	install $(STATIC_LIB) $(LIBDIR)
	install src/jslex.h $(INCLUDE)
	mkdir -p $(TEMPLATE_PATH)
	install templates/*.lua $(TEMPLATE_PATH)

# vi: noet sw=8 ts=8 tw=80

