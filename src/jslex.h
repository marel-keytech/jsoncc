#ifndef JSLEX_H_INCLUDED_
#define JSLEX_H_INCLUDED_

enum jslex_token_type {
    JSLEX_LITERAL,
    JSLEX_EQ,
    JSLEX_DOT,
    JSLEX_QMARK,
    JSLEX_PIPE,
    JSLEX_INTEGER,
    JSLEX_STRING,
    JSLEX_REGEX,
    JSLEX_REAL,
    JSLEX_COMMA,
    JSLEX_COLON,
    JSLEX_SEMICOMMA,
    JSLEX_LPAREN,
    JSLEX_RPAREN,
    JSLEX_LBRACKET,
    JSLEX_RBRACKET,
    JSLEX_LBRACE,
    JSLEX_RBRACE,
    JSLEX_EOF
};

struct jslex_token {
    enum jslex_token_type type;
    union {
        char* str;
        long long integer;
        double real;
    } value;
};

struct jslex {
    struct jslex_token current_token;
    const char* input;
    const char* pos;
    const char* next_pos;
    const char* line_start;
    int current_line;
    char* buffer;
    size_t buffer_size;
    int accepted;
    int errno_;
};

int jslex_init(struct jslex* self, const char* input);
void jslex_cleanup(struct jslex* self);

struct jslex_token* jslex_next_token(struct jslex* self);
void jslex_accept_token(struct jslex* self);

#endif /* JSLEX_H_INCLUDED_ */

