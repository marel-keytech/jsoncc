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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#include "jslex.h"

int jslex_init(struct jslex* self, const char* input)
{
    memset(self, 0, sizeof(*self));

    self->input = input;
    self->pos = input;
    self->accepted = 1;
    self->current_line = 1;
    self->line_start = input;

    /* TODO: Make buffer dynamic? */
    self->buffer_size = strlen(input) + 1;
    self->buffer = malloc(self->buffer_size);
    if(!self->buffer)
        return -1;

    return 0;
}

void jslex_cleanup(struct jslex* self)
{
    free(self->buffer);
}

void skip_whitespace(struct jslex* self)
{
    int i;
    const char* pos = self->pos;

    for(i = 0; pos[i]; ++i)
    {
        if(!isspace(pos[i]))
            break;

        if(pos[i] == '\n')
        {
            self->line_start = &pos[i+1];
            self->current_line++;
        }
    }

    self->pos += i;
}

static inline int is_literal_char(char c)
{
    return isalnum(c) || c == '_';
}

size_t get_literal_length(const char* literal)
{
    if (!isalpha(*literal) || *literal == '_')
        return 0;

    size_t len;
    for(len = 1; literal[len] && is_literal_char(literal[len]); ++len);

    return len;
}

void copy_literal(struct jslex* self)
{
    size_t len = get_literal_length(self->pos);
    memcpy(self->buffer, self->pos, len);
    self->buffer[len] = 0;
    self->current_token.value.str = self->buffer;
}

int classify_number(struct jslex* self)
{
    double real;
    long long integer;
    char* endptr;

    size_t real_len, integer_len;

    errno = 0;
    endptr = 0;
    real = strtod(self->pos, &endptr);
    real_len = errno ? 0 : endptr - self->pos;
    if(errno)
        self->errno_ = errno;

    errno = 0;
    endptr = 0;
    integer = strtoll(self->pos, &endptr, 0);
    integer_len = errno ? 0 : endptr - self->pos;
    if(errno)
        self->errno_ = errno;

    if(real_len == 0 && integer_len == 0)
        return -1;

    if(real_len > integer_len)
    {
        self->current_token.type = JSLEX_REAL;
        self->current_token.value.real = real;
        self->next_pos = self->pos + real_len;
    }
    else
    {
        self->current_token.type = JSLEX_INTEGER;
        self->current_token.value.integer = integer;
        self->next_pos = self->pos + integer_len;
    }

    return 0;
}

int classify_string(struct jslex* self)
{
    assert(*self->pos == '"');

    char* dst = self->buffer;
    const char* src = self->pos;

    int is_escaped = 0;

    int i;
    for(i = 1; src[i]; ++i)
    {
        if(is_escaped)
        {
            switch(src[i])
            {
            case '"':
                *dst++ = '"';
                break;
            case '\\':
                *dst++ = '\\';
                break;
            case 'n':
                *dst++ = '\n';
                break;
            case 't':
                *dst++ = '\t';
                break;
            case 'b':
                *dst++ = '\b';
                break;
            case 'f':
                *dst++ = '\f';
                break;
            case 'r':
                *dst++ = '\r';
                break;
            default:
                goto error;
            }

            is_escaped = 0;
        }
        else
        {
            switch(src[i])
            {
            case '"':
                goto done;
            case '\\':
                is_escaped = 1;
                break;
            case '\n':
                self->line_start = &src[i+1];
                self->current_line++;
            default:
                *dst++ = src[i];
                break;
            }
        }
    }

error:
    self->pos += i;
    return -1;

done:
    *dst = 0;
    self->current_token.type = JSLEX_STRING;
    self->current_token.value.str = self->buffer;
    self->next_pos = self->pos + i + 1;

    return 0;
}

int classify_regex(struct jslex* self)
{
    assert(*self->pos == '/');

    char* dst = self->buffer;
    const char* src = self->pos;

    int is_escaped = 0;

    int i;
    for(i = 1; src[i]; ++i)
    {
        if(is_escaped)
        {
            switch(src[i])
            {
            case '/':
                *dst++ = '/';
                break;
            default:
                *dst++ = '\\';
                *dst++ = src[i];
                break;
            }

            is_escaped = 0;
        }
        else
        {
            switch(src[i])
            {
            case '/':
                goto done;
            case '\\':
                is_escaped = 1;
                break;
            default:
                *dst++ = src[i];
                break;
            }
        }
    }

    self->pos += i;
    return -1;

done:
    *dst = 0;
    self->current_token.type = JSLEX_REGEX;
    self->current_token.value.str = self->buffer;
    self->next_pos = self->pos + i + 1;

    return 0;
}

int classify_token(struct jslex* self)
{
    while(*self->pos == '#')
    {
        self->pos += strcspn(self->pos, "\n");
        if(*self->pos == '\n')
        {
            self->pos++;
            self->line_start = self->pos;
            self->current_line++;
        }
    }

    switch(*self->pos)
    {
    case '.':
        self->current_token.type = JSLEX_DOT;
        self->next_pos = self->pos + 1;
        return 0;
    case '?':
        self->current_token.type = JSLEX_QMARK;
        self->next_pos = self->pos + 1;
        return 0;
    case '=':
        self->current_token.type = JSLEX_EQ;
        self->next_pos = self->pos + 1;
        return 0;
    case '|':
        self->current_token.type = JSLEX_PIPE;
        self->next_pos = self->pos + 1;
        return 0;
    case ',':
        self->current_token.type = JSLEX_COMMA;
        self->next_pos = self->pos + 1;
        return 0;
    case ':':
        self->current_token.type = JSLEX_COLON;
        self->next_pos = self->pos + 1;
        return 0;
    case ';':
        self->current_token.type = JSLEX_SEMICOMMA;
        self->next_pos = self->pos + 1;
        return 0;
    case '(':
        self->current_token.type = JSLEX_LPAREN;
        self->next_pos = self->pos + 1;
        return 0;
    case ')':
        self->current_token.type = JSLEX_RPAREN;
        self->next_pos = self->pos + 1;
        return 0;
    case '[':
        self->current_token.type = JSLEX_LBRACKET;
        self->next_pos = self->pos + 1;
        return 0;
    case ']':
        self->current_token.type = JSLEX_RBRACKET;
        self->next_pos = self->pos + 1;
        return 0;
    case '{':
        self->current_token.type = JSLEX_LBRACE;
        self->next_pos = self->pos + 1;
        return 0;
    case '}':
        self->current_token.type = JSLEX_RBRACE;
        self->next_pos = self->pos + 1;
        return 0;
    case '"':
        return classify_string(self);
    case '/':
        return classify_regex(self);
    case '\0':
        self->current_token.type = JSLEX_EOF;
        self->next_pos = self->pos + 1;
        return 0;
    default:
        break;
    }

    if(isalpha(*self->pos) || *self->pos == '_')
    {
        self->current_token.type = JSLEX_LITERAL;
        copy_literal(self);
        self->next_pos = self->pos + get_literal_length(self->pos);
        return 0;
    }

    if(classify_number(self) >= 0)
        return 0;

    return -1;
}

struct jslex_token* jslex_next_token(struct jslex* self)
{
    if(self->current_token.type == JSLEX_EOF)
        return &self->current_token;

    if(!self->accepted)
        return &self->current_token;

    if(self->next_pos)
        self->pos = self->next_pos;

    skip_whitespace(self);

    if(classify_token(self) < 0)
        return NULL;

    self->accepted = 0;

    return &self->current_token;
}

void jslex_accept_token(struct jslex* self)
{
    self->accepted = 1;
}

