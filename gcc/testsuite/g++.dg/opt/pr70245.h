extern struct A *a, *i;
extern int b, c, e, l;
int *fn1 (char *, int *);
void fn2 ();
void *fn3 (int *);
struct B { char *b; };
typedef void (*F) (A *, B *, unsigned char *, int *);
struct C { int c[16]; };
struct D { int d; };
struct A { D a1; C a2; };
void *fn4 ();
extern F d;
extern B k;
extern void baz (int);
