%token_type { char* }
%extra_argument { struct obj_state* state }
%type decl { struct obj* }
%type declarations { struct obj* }

%syntax_error {
    fprintf(stderr, "Warning: Bad syntax in line %d\n", state->line);
}

//%parse_failed {
//    fprintf(stderr, "Error: Failed to parse file\n");
//}

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

main ::= START declarations(Obj) END.  {
     printf("main\n");
    state->obj = Obj;
}

declarations(R) ::= decl(Obj).  {
     printf("declarations 1\n");
    R = Obj;
}

declarations(R) ::= decl(Head) declarations(Tail).  {
     printf("declarations 2\n");
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

