/*
 * Copyright (c) 2015, Marel hf
 * Copyright (c) 2015, Andri Yngvason
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* Grammar:
 * members <- member+
 * member <- name ':' type length? term
 * name <- literal
 * type <- object / 'real' / 'integer' / 'bool' / 'any'
 * length <- '[' integer? ']'
 * term <- '.' | '?'
 * object <- '{' members '}'
 */

#include <stdlib.h>
#include "jslex.h"
#include "obj.h"

#define EXPECT_STACK_MAX_SIZE 128

enum error_type {
    E_UNKNOWN = 0,
    E_OOM,
    E_UNKNOWN_TOKEN,
    E_UNEXPECTED_TOKEN
};

static struct jslex lexer_;
struct jslex_token* token_;

static enum error_type error_;

static int expect_stack_index_ = 0;
static enum jslex_token_type expect_stack_[EXPECT_STACK_MAX_SIZE];

static inline void expect_stack_push(enum jslex_token_type type)
{
    if(expect_stack_index_ < EXPECT_STACK_MAX_SIZE)
        expect_stack_[expect_stack_index_++] = type;
}

static inline void next_token()
{
    token_ = jslex_next_token(&lexer_);
}

static inline int accept_token()
{
    error_ = 0;
    expect_stack_index_ = 0;
    jslex_accept_token(&lexer_);
    return 1;
}

static int expect(enum jslex_token_type type)
{
    next_token();
    if(!token_)
    {
        error_ = E_UNKNOWN_TOKEN;
        return 0;
    }

    if(token_->type != type)
    {
        error_ = E_UNEXPECTED_TOKEN;
        expect_stack_push(type);
        return 0;
    }

    return 1;
}

static int name(struct obj* obj)
{
    if(!expect(JSLEX_LITERAL))
        return 0;

    obj_set_name(obj, token_->value.str);

    return accept_token();
}

static inline int colon()    { return expect(JSLEX_COLON) && accept_token(); }
static inline int dot()      { return expect(JSLEX_DOT)   && accept_token(); }

static inline int lbracket()
{
    return expect(JSLEX_LBRACKET) && accept_token();
}

static inline int rbracket()
{
    return expect(JSLEX_RBRACKET) && accept_token();
}

static inline int lbrace()
{
    return expect(JSLEX_LBRACE) && accept_token();
}

static inline int rbrace()
{
    return expect(JSLEX_RBRACE) && accept_token();
}

static int length(struct obj* obj)
{
    if(expect(JSLEX_INTEGER))
    {
        obj->length = token_->value.integer;
        accept_token();
    }
    else
    {
        obj->length = -1;
    }

    return rbracket();
}

static int qmark(struct obj* obj)
{
    if(!expect(JSLEX_QMARK))
        return 0;

    obj->is_optional = 1;

    return accept_token();
}

static inline int decls(struct obj* obj);

static int literal_type(struct obj* obj)
{
    if(!expect(JSLEX_LITERAL))
        return 0;

    if(0 == strcmp("string", token_->value.str))
        obj->type = OBJ_STRING;
    else if(0 == strcmp("int", token_->value.str))
        obj->type = OBJ_INTEGER;
    else if(0 == strcmp("real", token_->value.str))
        obj->type = OBJ_REAL;
    else if(0 == strcmp("bool", token_->value.str))
        obj->type = OBJ_BOOL;
    else if(0 == strcmp("any", token_->value.str))
        obj->type = OBJ_ANY;
    else
        return 0;

    return accept_token();
}

static int object(struct obj* obj)
{
    if(!lbrace())
       return 0;

    obj->type = OBJ_OBJECT;
    obj->children = obj_new();

    if(!(decls(obj->children) && rbrace()))
    {
        obj_free(obj->children);
        obj->children = NULL;
        return 0;
    }

    return 1;
}

static inline int type(struct obj* obj)
{
    return literal_type(obj) || object(obj);
}

static inline int decl(struct obj* obj)
{
    return name(obj)
        && colon()
        && type(obj)
        && (lbracket() ? length(obj) : 1)
        && (dot() || qmark(obj));
}

static int decls(struct obj* obj)
{
    if(!decl(obj))
        return 0;

    struct obj* next;

    while(1)
    {
        if(expect(JSLEX_RBRACE) || expect(JSLEX_EOF))
            break;

        if(!expect(JSLEX_LITERAL))
            return 0;

        next = obj_new();
        if(!decl(next))
        {
            obj_free(next);
            return 0;
        }

        obj->next = next;
        obj = next;
    }

    return 1;
}

static int desc(struct obj* obj)
{
    return decls(obj) && expect(JSLEX_EOF);
}

struct obj* desc_parse(const char* input)
{
    struct obj* obj = obj_new();

    if(jslex_init(&lexer_, input) < 0)
        goto failure;

    if(!desc(obj))
        goto failure;

    jslex_cleanup(&lexer_);
    return obj;

failure:
    obj_free(obj);
    return NULL;
}

