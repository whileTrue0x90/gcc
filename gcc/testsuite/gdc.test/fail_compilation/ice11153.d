/*
TEST_OUTPUT:
---
fail_compilation/ice11153.d(11): Error: function declaration without return type. (Note that constructors are always named `this`)
fail_compilation/ice11153.d(11): Error: no identifier for declarator `foo()`
---
*/

struct S
{
    foo(T)() {}
    // Parser creates a TemplateDeclaration object with ident == NULL
}

void main() {}
