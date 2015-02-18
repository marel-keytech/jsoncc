%token_type { char* }
%extra_argument { struct obj_state* state }
%type decl { struct obj* }
%type declarations { struct obj* }

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

declarations(R) ::= decl(Obj).  {
    R = Obj;
}

declarations(R) ::= decl(Head) declarations(Tail).  {
    Head->next = Tail;
    R = Head;
}

decl(R) ::= OBJECT NAME(Name) LBRACE declarations(Children) RBRACE DOT.  {
    R = obj_obj_new(Name, Children);
}

decl(R) ::= STRING NAME(Name) DOT.  {
    R = obj_new(STRING, Name, 1);
}

decl(R) ::= STRING NAME(Name) LENGTH(Len) DOT.  {
    R = obj_new(STRING, Name, atoi(Len));
}

decl(R) ::= INTEGER NAME(Name) DOT.  {
    R = obj_new(INTEGER, Name, 1);
}

decl(R) ::= INTEGER NAME(Name) LENGTH(Len) DOT.  {
    R = obj_new(INTEGER, Name, atoi(Len));
}

decl(R) ::= REAL NAME(Name) DOT.  {
    R = obj_new(REAL, Name, 1);
}

decl(R) ::= REAL NAME(Name) LENGTH(Len) DOT.  {
    R = obj_new(REAL, Name, atoi(Len));
}

decl(R) ::= BOOL NAME(Name) DOT.  {
    R = obj_new(BOOL, Name, 1);
}

decl(R) ::= BOOL NAME(Name) LENGTH(Len) DOT.  {
    R = obj_new(BOOL, Name, atoi(Len));
}

