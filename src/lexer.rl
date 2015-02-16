#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "obj.h"

void *ParseAlloc();
void Parse();
void ParseFree();

static char* p = 0;
static char* _mark = 0;
static void* _parser = 0; 
static struct obj_state _state = { 0 };

void mark()
{
    _mark = p;
}

void emit(int token)
{
    if(_mark)
        *p = 0;
    printf("emit: %d %s\n", token, _mark ? _mark : "");
    Parse(_parser, token, _mark, &_state);
    _mark = NULL;
}

struct obj* lexer(char* data)
{
    p = data;
    char* pe = p + strlen(p);
    char* eof = pe;
    int cs;

    _parser = ParseAlloc(malloc);

    emit(START);

    %%{
        machine lexer;
        write data;

        action mark
        {
            mark();
        }

        newline = '\n' @{ _state.line++; };
        ws = [\t ] | newline;

        lbrace = '{' @{ emit(LBRACE); };
        rbrace = '}' @{ emit(RBRACE); };
        dot = '.' @{ emit(DOT); };
        length = '[' (digit+ >mark %{ emit(LENGTH); }) ']';

        string = 'string' @{ emit(STRING); };
        int = 'int' @{ emit(INTEGER); };
        real = 'real' @{ emit(REAL); };
        object = 'object' @{ emit(OBJECT); };
        type = string | int | real | object;

        name = (alpha (alnum | '_')*) >mark %{ emit(NAME); };

        decl = type ws* name ws* length?;

        comment = '#' (any - '\n')*;

        token = ws | decl | lbrace | rbrace | comment | dot;

        main := token**;

        write init;
        write exec;
    }%%

    emit(END);

    ParseFree(_parser, free);
    return _state.obj;
}

