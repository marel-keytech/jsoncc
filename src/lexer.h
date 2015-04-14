#ifndef LEXER_H_INCLUDED_
#define LEXER_H_INCLUDED_

enum token_type {
    TOK_LITERAL,
    TOK_EQ,
    TOK_PIPE,
    TOK_INTEGER,
    TOK_STRING,
    TOK_REGEX,
    TOK_REAL,
    TOK_COMMA,
    TOK_COLON,
    TOK_SEMICOMMA,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_EOF
};

struct token {
    enum token_type type;
    union {
        char* str;
        long long integer;
        double real;
    } value;
};

struct lexer {
    struct token current_token;
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

int lexer_init(struct lexer* self, const char* input);
void lexer_cleanup(struct lexer* self);

struct token* lexer_next_token(struct lexer* self);

void lexer_accept_token(struct lexer* self);

#endif /* LEXER_H_INCLUDED_ */

