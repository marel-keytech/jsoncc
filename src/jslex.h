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

enum json_obj_type {
    JSON_OBJ_NULL = 0,
    JSON_OBJ_OBJECT,
    JSON_OBJ_ARRAY,
    JSON_OBJ_REAL,
    JSON_OBJ_INTEGER,
    JSON_OBJ_STRING,
    JSON_OBJ_BOOL
};

struct json_obj_any {
    enum json_obj_type type;
    long long integer;
    double real;
    char* string_;
    int boolean;
};

int jslex_init(struct jslex* self, const char* input);
void jslex_cleanup(struct jslex* self);

struct jslex_token* jslex_next_token(struct jslex* self);
void jslex_accept_token(struct jslex* self);

char* json_string_decode(const char* input, size_t len);
char* json_string_encode(const char* input, size_t len);

#endif /* JSLEX_H_INCLUDED_ */

