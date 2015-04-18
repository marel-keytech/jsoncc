# jsoncc - JSON Compiler Compiler

Jsoncc generates C code that maps C structures to JSON and vice versa.

The specification format looks similar to JSON. See tst/test.x for an example.

## Features
* Types: integer, real, boolean, string, object, array and any.
* Decoding of json into a predefined structure.
* Encoding of a structure into json.
* Validation according to the specification.
  * Objects may contain unused members.
  * Optional members can be specified using the question mark.

## Limitations
* Member names have the same restrictions as C variable names.
* All members of an array must be of the same type.
* Only ASCII is supported.

## TODO
* Support static arrays (foo: int[42] fails as is).
* Reject duplicates
* Do something about 'nil'
* Add 'any' arrays
* Add 'object' arrays
* Add array-arrays
* Generate useful error messages when compilation fails.

## Dependencies:
* Lua/LuaJIT 5.1. Note: The generated code does NOT depend on Lua.
* gcc/clang

## Implementation Details
The generated parser is a recursive descent parser that writes its results
straight into the structure, so a structure that does not contain any strings
does not require any memory allocation. The parser uses the lexer found in
jslex.c, so you must link to the lexer.

