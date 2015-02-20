%name jsonParse
%token_type { struct json_obj_pos }
%token_prefix TOK_
%extra_argument { struct json_obj_state* state }
%default_type { struct json_obj* }
%default_destructor { json_obj_free($$); }
%destructor root { }

%syntax_error {
    fprintf(stderr, "Error: Bad syntax in line %d\n", state->line+1);
}

%include {
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "json_obj.h"
}

%right LBRACE LBRACKET.
%left RBRACE RBRACKET.
//%nonassoc COLON.
%right COMMA.

main ::= LBRACE root RBRACE.
root ::= kvpairs(Obj). { state->obj = Obj; }

object(R) ::= NUMBER(Number). {
    R = json_obj_new(JSON_OBJ_NUMBER, Number.start, Number.end);
}

object(R) ::= STRING(String). {
    R = json_obj_new(JSON_OBJ_STRING, String.start, String.end);
}

object(R) ::= LBRACKET(Left) objects(Obj) RBRACKET(Right). {
    R = json_obj_new_obj(JSON_OBJ_ARRAY, Obj, Left.start, Right.end);
}

object(R) ::= LBRACE(Left) kvpairs(Obj) RBRACE(Right). {
    R = json_obj_new_obj(JSON_OBJ_OBJ, Obj, Left.start, Right.end);
}

object(R) ::= LBRACKET(Left) RBRACKET(Right). {
    R = json_obj_new(JSON_OBJ_NULL, Left.start, Right.end);
}

object(R) ::= LBRACE(Left) RBRACE(Right). {
    R = json_obj_new(JSON_OBJ_NULL, Left.start, Right.end);
}

object(R) ::= NULL(Value). {
    R = json_obj_new(JSON_OBJ_NULL, Value.start, Value.end);
}

object(R) ::= TRUE(Value). {
    R = json_obj_new(JSON_OBJ_TRUE, Value.start, Value.end);
}

object(R) ::= FALSE(Value). {
    R = json_obj_new(JSON_OBJ_FALSE, Value.start, Value.end);
}

kvpair(R) ::= STRING(Key) COLON object(Value). {
    json_obj_set_key(Value, Key); R = Value;
}

kvpairs(R) ::= kvpair(Pair). { R = Pair; }
kvpairs(R) ::= kvpair(Head) COMMA kvpairs(Tail). {
    Head->next = Tail;
    R = Head;
}

objects(R) ::= object(Obj). { R = Obj; }
objects(R) ::= object(Head) COMMA objects(Tail). {
    Head->next = Tail;
    R = Head;
}

