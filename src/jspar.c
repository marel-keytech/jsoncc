/* Grammar according to http://rfc7159.net/

JSON-text = ws value ws
begin-array     = ws %x5B ws  ; [ left square bracket
begin-object    = ws %x7B ws  ; { left curly bracket
end-array       = ws %x5D ws  ; ] right square bracket
end-object      = ws %x7D ws  ; } right curly bracket
name-separator  = ws %x3A ws  ; : colon
value-separator = ws %x2C ws  ; , comma

ws = *(
       %x20 /              ; Space
       %x09 /              ; Horizontal tab
       %x0A /              ; Line feed or New line
       %x0D )              ; Carriage return

value = false / null / true / object / array / number / string
false = %x66.61.6c.73.65   ; false
null  = %x6e.75.6c.6c      ; null
true  = %x74.72.75.65      ; true

object = begin-object [ member *( value-separator member ) ] end-object
member = string name-separator value

array = begin-array [ value *( value-separator value ) ] end-array

number = [ minus ] int [ frac ] [ exp ]
decimal-point = %x2E       ; .
digit1-9 = %x31-39         ; 1-9
e = %x65 / %x45            ; e E
exp = e [ minus / plus ] 1*DIGIT
frac = decimal-point 1*DIGIT
int = zero / ( digit1-9 *DIGIT )
minus = %x2D               ; -
plus = %x2B                ; +
zero = %x30                ; 0

string = quotation-mark *char quotation-mark

char = unescaped /
   escape (
       %x22 /          ; "    quotation mark  U+0022
       %x5C /          ; \    reverse solidus U+005C
       %x2F /          ; /    solidus         U+002F
       %x62 /          ; b    backspace       U+0008
       %x66 /          ; f    form feed       U+000C
       %x6E /          ; n    line feed       U+000A
       %x72 /          ; r    carriage return U+000D
       %x74 /          ; t    tab             U+0009
       %x75 4HEXDIG )  ; uXXXX                U+XXXX

escape = %x5C              ; \
quotation-mark = %x22      ; "
unescaped = %x20-21 / %x23-5B / %x5D-10FFFF

Description:
foo: int.
bar: string.
puff: any.
x: { y: real. z: real. }.
dragon: int[].
*/

#include <string.h>
#include <stdlib.h>
#include "jslex.h"

struct my_struct {
    int foo;
    int bar;
    struct json_obj_any puff;
    struct {
        double y;
    } x;
    size_t length_of_dragon;
    size_t reserved_size_of_dragon;
    long long* dragon;
};

int my_lbracket(struct jslex* lexer)
{
    return 1;
}

int my_rbracket(struct jslex* lexer)
{
    return 1;
}

int my_lbrace(struct jslex* lexer)
{
    return 1;
}

int my_rbrace(struct jslex* lexer)
{
    return 1;
}

int my_comma(struct jslex* lexer)
{
    return 1;
}

int my_colon(struct jslex* lexer)
{
    return 1;
}

int my_key(struct jslex* lexer, const char* key)
{
    struct jslex_token* tok = jslex_next_token(lexer);
    if(!tok)
        return 0;

    if(tok->type != JSLEX_STRING)
        return 0;

    if(0 != strcmp(key, tok->value.str))
        return 0;

    jslex_accept_token(lexer);

    return 1;
}

int my_junk_token(struct jslex* lexer, enum jslex_token_type type)
{
    struct jslex_token* tok = jslex_next_token(lexer);
    if(tok)
        return 0;

    if(tok->type != type)
        return 0;

    jslex_accept_token(lexer);
    return 1;
}

int my_junk_integer(struct jslex* lexer)
{
    return my_junk_token(lexer, JSLEX_INTEGER);
}

int my_junk_real(struct jslex* lexer)
{
    return my_junk_token(lexer, JSLEX_REAL);
}

int my_junk_literal(struct jslex* lexer)
{
    return my_junk_token(lexer, JSLEX_LITERAL);
}

int my_junk_string(struct jslex* lexer)
{
    return my_junk_token(lexer, JSLEX_STRING);
}

int my_junk_values(struct jslex* lexer);

int my_junk_array(struct jslex* lexer)
{
    return my_lbracket(lexer) && (my_rbracket(lexer) || (
                my_junk_values(lexer) && my_rbracket(lexer)));
}

int my_junk_value(struct jslex* lexer);

int my_junk_member(struct jslex* lexer)
{
    return my_junk_string(lexer) && my_colon(lexer) && my_junk_value(lexer);
}

int my_junk_members(struct jslex* lexer)
{
    return my_junk_member(lexer)
        && (my_comma(lexer) ? my_junk_members(lexer) : 1);
}

int my_junk_object(struct jslex* lexer)
{
    return my_lbrace(lexer) && (my_rbrace(lexer) || (
                my_junk_members(lexer) && my_rbrace(lexer)));
}

int my_junk_value(struct jslex* lexer)
{
    return my_junk_integer(lexer)
        || my_junk_real(lexer)
        || my_junk_literal(lexer)
        || my_junk_string(lexer)
        || my_junk_array(lexer)
        || my_junk_object(lexer);
}

int my_junk_values(struct jslex* lexer)
{
    return my_junk_value(lexer)
        && (my_comma(lexer) ? my_junk_values(lexer) : 1);
}

int my_root__foo_value(struct my_struct* dest, struct jslex* lexer)
{
    struct jslex_token* tok = jslex_next_token(lexer);
    if(!tok)
        return 0;

    if(tok->type != JSLEX_INTEGER)
        return 0;

    dest->foo = tok->value.integer;

    return 1;
}

int my_root__foo(struct my_struct* dest, struct jslex* lexer)
{
    return my_key(lexer, "foo")
        && my_colon(lexer)
        && my_root__foo_value(dest, lexer);
}

int my_root__bar_value(struct my_struct* dest, struct jslex* lexer)
{
    struct jslex_token* tok = jslex_next_token(lexer);
    if(!tok)
        return 0;

    if(tok->type != JSLEX_INTEGER)
        return 0;

    dest->bar = tok->value.integer;

    return 1;
}

int my_root__bar(struct my_struct* dest, struct jslex* lexer)
{
    return my_key(lexer, "bar")
        && my_colon(lexer)
        && my_root__bar_value(dest, lexer);
}

int my_root__x_members(struct my_struct* dest, struct jslex* lexer)
{
    return my_root__x__y(dest, lexer)
        || my_root__x__z(dest, lexer)
        || my_junk_value(lexer);
}

int my_root__x_value(struct my_struct* dest, struct jslex* lexer)
{
    return my_lbrace(lexer) && (
            my_rbrace(lexer) || (
                my_root__x_members(dest, lexer) && my_rbrace(lexer)
            ));
}

int my_root__x(struct my_struct* dest, struct jslex* lexer)
{
    return my_key(lexer, "x")
        && my_colon(lexer)
        && my_root__x_value(dest, lexer);
}

int my_any_integer(struct json_obj_any* any, struct jslex* lexer)
{
    struct jslex_token* tok = jslex_next_token(lexer);
    if(tok)
        return 0;

    if(tok->type != JSLEX_INTEGER)
        return 0;

    any->type = JSON_OBJ_INTEGER;
    any->integer = tok->value.integer;

    jslex_accept_token(lexer);
    return 1;
}

int my_any_value(struct my_struct* dest, struct json_obj_any* any,
                 struct jslex* lexer)
{
    return my_any_integer(any, lexer)
        || my_any_real(any, lexer)
        || my_any_string(any, lexer)
        || my_any_bool(any, lexer);
}

int my_root__puff(struct my_struct* dest, struct jslex* lexer)
{
    return my_key(lexer, "puff")
        && my_colon(lexer)
        && my_any_value(dest, &dest->puff, lexer);
}

static ssize_t my_grow_dragon(struct my_struct* self, size_t new_size)
{
    if(new_size <= self->reserved_size_of_dragon)
        return 0;
    else
        self->reserved_size_of_dragon = new_size*2;

    self->dragon = realloc(self->dragon, self->reserved_size_of_dragon);
    if(!self->dragon)
        return -1;

    return self->reserved_size_of_dragon;
}

static ssize_t my_append_to_dragon(struct my_struct* self, long long elem)
{
    if(my_grow_dragon(self, self->length_of_dragon + 1) < 0)
        return -1;

    self->dragon[self->length_of_dragon++] = elem;

    return 1;
}
int my__dragon_value(struct my_struct* dest, struct jslex* lexer)
{
    struct jslex_token* tok = jslex_next_token(lexer);
    if(!tok)
        return 0;

    if(tok->type != JSLEX_INTEGER)
        return 0;

    if(my_append_to_dragon(dest, tok->value.integer) < 0)
        return 0;

    jslex_accept_token(lexer);
    return 1;
}

int my__dragon_values(struct my_struct* dest, struct jslex* lexer)
{
    return my__dragon_value(dest, lexer)
        && (my_comma(lexer) ? my__dragon_values(dest, lexer) : 1);
}

int my__dragon_array(struct my_struct* dest, struct jslex* lexer)
{
    return my_lbracket(lexer) && (my_rbracket(lexer) || (
                my__dragon_values(dest, lexer) && my_rbracket(lexer)));
}

int my_root__dragon(struct my_struct* dest, struct jslex* lexer)
{
    return my_key(lexer, "dragon")
        && my_colon(lexer)
        && my__dragon_array(dest, lexer);
}

int my_root_member(struct my_struct* dest, struct jslex* lexer)
{
    return my_root__foo(dest, lexer)
        || my_root__bar(dest, lexer)
        || my_root__x(dest, lexer)
        || my_root__puff(dest, lexer)
        || my_junk_value(lexer);
}

int my_root_members(struct my_struct* dest, struct jslex* lexer)
{
    return my_root_member(dest, lexer)
        && (my_comma(lexer) ? my_root_members(dest, lexer) : 1);
}

int my_root_object(struct my_struct* dest, struct jslex* lexer)
{
    return my_lbrace(lexer) && (my_rbrace(lexer) || (
                my_root_members(dest, lexer) && my_rbrace(lexer)));
}

int my_json(struct my_struct* dest, const char* input)
{
    struct jslex lexer;

    jslex_init(&lexer, input);

    int r = my_root_object(dest, &lexer);

    jslex_cleanup(&lexer);

    return r;
}

