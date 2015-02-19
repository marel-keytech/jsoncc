%token_type { char* }
%extra_argument { struct obj_state* state }
%type decl { struct obj* }
%type declarations { struct obj* }
%type full_decl { struct obj* }
%type type { int }

%syntax_error {
    fprintf(stderr, "Error: Bad syntax in line %d\n", state->line + 1);
}

%include {
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "obj.h"
}

%right START.
%left END.
%right OBJECT INT REAL STRING.
%right NAME.
%right LBRACE.
%left RBRACE.

main ::= START program END.

program ::= declarations(Obj). {
    state->obj = Obj;
}

declarations(R) ::= full_decl(Obj).  {
    R = Obj;
}

declarations(R) ::= full_decl(Head) declarations(Tail).  {
    Head->next = Tail;
    R = Head;
}

full_decl(R) ::= decl(Obj) DOT.   { R = Obj; }
full_decl(R) ::= decl(Obj) QMARK. { obj_make_optional(Obj); R = Obj; }

decl(R) ::= NAME(Name) COLON type(Type). {
    R = obj_new(Type, Name, 1);
}

decl(R) ::= NAME(Name) COLON type(Type) LENGTH(Length). {
    R = obj_new(Type, Name, atoi(Length));
}

decl(R) ::= NAME(Name) COLON LBRACE declarations(Children) RBRACE. {
    R = obj_obj_new(Name, Children);
}

type(R) ::= STRING.  { R = STRING; }
type(R) ::= INTEGER. { R = INTEGER; }
type(R) ::= REAL.    { R = REAL; }
type(R) ::= BOOL.    { R = BOOL; }
type(R) ::= ANY.     { R = ANY; }
