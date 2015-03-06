#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "json_string.h"

struct buffer {
    char* buffer;
    size_t size;
    int index;
};

static ssize_t buffer_grow(struct buffer* self, size_t size)
{
    if(size < self->size)
        return 0;
    else
        self->size = size*2;

    self->buffer = malloc(self->size);
    if(!self->buffer)
        return -1;

    return self->size;
}

static ssize_t buffer_append(struct buffer* self, char c)
{
    if(buffer_grow(self, self->index+1) < 0)
        return -1;
    
    self->buffer[self->index++] = c;

    return 1;
}

static ssize_t buffer_append_str(struct buffer* self, const char* str, size_t len)
{
    if(buffer_grow(self, self->index+len) < 0)
        return -1;
    
    memcpy(&self->buffer[self->index], str, len);
    self->index += len;

    return (ssize_t)len;
}

static inline int convertdigit(char digit)
{
    return isdigit(digit) ? digit - '0' : tolower(digit) - 'a';
}

static ssize_t decode_unicode(struct buffer* output, const char* input)
{
    if(!(input[0] && input[1] && input[2] && input[3]))
        return 1;

    if(!(isxdigit(input[0]) && isxdigit(input[1]) &&
         isxdigit(input[2]) && isxdigit(input[3])))
        return 1;

    unsigned int code = convertdigit(input[0]) << 12
                      | convertdigit(input[1]) << 8
                      | convertdigit(input[2]) << 4
                      | convertdigit(input[3]);

    /* only ascii is supported for now */
    if(code <= 0xff)
        if(buffer_append(output, code) < 0)
            return -1;

    return 5;
}

static ssize_t decode_escape(struct buffer* output, const char* input)
{
    switch(*input)
    {
    case 0:   return 0;
    case 'b': return buffer_append(output, '\b');
    case 'f': return buffer_append(output, '\f');
    case 'n': return buffer_append(output, '\n');
    case 'r': return buffer_append(output, '\r');
    case 't': return buffer_append(output, '\t');
    case 'u': return decode_unicode(output, input+1);
    default:  return buffer_append(output, *input);
    }
}

char* json_string_decode(const char* input)
{
    ssize_t sz;
    struct buffer output;
    memset(&output, 0, sizeof(output));

    if(buffer_grow(&output, strlen(input)+1) < 0)
        return NULL;

    while(*input)
    {
        sz = *input != '\\' ? buffer_append(&output, *input)
                            : decode_escape(&output, ++input);

        if(sz < 0)
            return NULL;

        input += sz;
    }

    if(buffer_append(&output, 0) < 0)
        return NULL;

    return output.buffer;
}

static inline char get_hexdigit(int number)
{
    const char lookup[] = "0123456789abcdef";
    return lookup[number];
}

static int encode_nonprint(struct buffer* output, char c)
{
    char str[6] = {
        '\\', 'u', '0', '0',
        get_hexdigit((c & 0xf0) >> 8),
        get_hexdigit(c & 0xf)
    };

    if(buffer_append_str(output, str, sizeof(str)) < 0)
        return 1;

    return 1;
}

static int encode_character(struct buffer* output, char c)
{
    switch(c)
    {
    case '\b': return buffer_append_str(output, "\\b", 2);
    case '\f': return buffer_append_str(output, "\\f", 2);
    case '\n': return buffer_append_str(output, "\\n", 2);
    case '\r': return buffer_append_str(output, "\\r", 2);
    case '\t': return buffer_append_str(output, "\\t", 2);
    case '\\': return buffer_append_str(output, "\\\\", 2);
    case '"':  return buffer_append_str(output, "\\\"", 2);
    default:   return isprint(c) ? buffer_append(output, c)
                                 : encode_nonprint(output, c);
    }
}

char* json_string_encode(const char* input)
{
    struct buffer output;
    memset(&output, 0, sizeof(output));

    if(buffer_grow(&output, strlen(input)+1) < 0)
        return NULL;

    while(*input)
        if(encode_character(&output, *input++) < 0)
            return NULL;

    if(buffer_append(&output, 0) < 0)
        return NULL;

    return output.buffer;
}

