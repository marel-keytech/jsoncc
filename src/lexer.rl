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

    %%machine lexer;
    %%write data;
    (void)lexer_en_main;
    (void)lexer_error;
    (void)lexer_first_final;

    %%{
        action mark
        {
            mark();
        }

        newline = '\n' @{ _state.line++; };
        ws = (space-'\n') | newline;

        lbrace = '{' @{ emit(LBRACE); };
        rbrace = '}' @{ emit(RBRACE); };
        dot = '.' @{ emit(DOT); };
        qmark = '?' @{ emit(QMARK); };
        colon = ':' @{ emit(COLON); };
        length = '[' (digit+ >mark %{ emit(LENGTH); }) ']';

        string = 'string' @{ emit(STRING); };
        int = 'int' @{ emit(INTEGER); };
        real = 'real' @{ emit(REAL); };
        bool = 'bool' @{ emit(BOOL); };
        any_type = 'any' @{ emit(ANY); };
        type = string | int | real | bool | any_type;

        name = (alpha (alnum | '_')*) >mark %{ emit(NAME); };

        decl = name ws* colon ws* type? ws* length?;

        comment = '#' (^'\n')*;

        token = ws | decl | lbrace | rbrace | comment | dot | qmark;

        main := token**;

        write init;
        write exec;
    }%%

    emit(END);

    ParseFree(_parser, free);
    return _state.obj;
}

