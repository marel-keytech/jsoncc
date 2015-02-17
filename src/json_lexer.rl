#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json_parser.h"
#include "json_obj.h"

void *jsonParseAlloc();
void jsonParse();
void jsonParseFree();

static void* _parser = 0; 
static struct json_obj_state _state = { 0 };

void emit(int token, const char* s, char* ts, char* te)
{
    struct json_obj_pos pos = { .start = ts-s, .end = te-s };
    jsonParse(_parser, token, pos, &_state);
}

struct json_obj* json_lexer(const char* data)
{
    char* p = (char*)data;
    char* pe = p + strlen(p);
    char* eof = pe;
    int cs;
    char* ts = NULL;
    char* te = NULL;
    int act = 0;

    _parser = jsonParseAlloc(malloc);

    %%{
        machine lexer;
        write data;

        action mark
        {
            mark();
        }

        newline = '\n';

        escape = '\\';
        escape_seq = escape (["/bfnrt] | escape | ('u' [0-9a-fA-F]{4}));
        string = '"' ((any - ('"' | escape) | escape_seq)*) '"';

        sign = '-'|'+';
        decimals = '.' digit+;
        exponent = [eE] sign? digit+;

        number = sign? digit+ decimals? exponent?;

        main := |*
            space-newline;
            newline => { _state.line++; };
            '{'     => { emit(TOK_LBRACE,   data, ts, te); };
            '}'     => { emit(TOK_RBRACE,   data, ts, te); };
            '['     => { emit(TOK_LBRACKET, data, ts, te); };
            ']'     => { emit(TOK_RBRACKET, data, ts, te); };
            ','     => { emit(TOK_COMMA,    data, ts, te); };
            ':'     => { emit(TOK_COLON,    data, ts, te); };
            'true'  => { emit(TOK_TRUE,     data, ts, te); };
            'false' => { emit(TOK_FALSE,    data, ts, te); };
            'null'  => { emit(TOK_NULL,     data, ts, te); };
            string  => { emit(TOK_STRING,   data, ts+1, te-1); };
            number  => { emit(TOK_NUMBER,   data, ts, te); };
        *|;

        write init;
        write exec;
    }%%

    jsonParseFree(_parser, free);
    return _state.obj;
}

