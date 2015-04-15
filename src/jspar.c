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
x: { y: real. z: real. }.
*/

#include <string.h>
#include "jslex.h"


struct my_struct {
    int foo;
    int bar;
    struct {
        double y;
    } x;
};

int my_lbracket(struct jslex* lexer)
{
}

int my_rbracket(struct jslex* lexer)
{
}

int my_lbrace(struct jslex* lexer)
{
}

int my_rbrace(struct jslex* lexer)
{
}

int my_comma(struct jslex* lexer)
{
}

int my_colon(struct jslex* lexer)
{
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

int my_root_member(struct my_struct* dest, struct jslex* lexer)
{
    return my_root__foo(dest, lexer)
        || my_root__bar(dest, lexer)
        || my_root__x(dest, lexer)
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

