/* { dg-do run } */
/* { dg-options "-std=gnu99" } */


int main()
{
	int n = 1;

	struct foo { char x[++n]; } bar(void) { }

	if (2 != n)
		__builtin_abort();

	if (2 != sizeof(bar()))
		__builtin_abort();

	n = 1;

	struct bar { char x[++n]; } (*bar2)(void) = bar;	/* { dg-warning "incompatible pointer type" } */

	if (2 != n)
		__builtin_abort();

	if (2 != sizeof((*bar2)()))
		__builtin_abort();

	n = 1;

	struct { char x[++n]; } *bar3(void) { }

	if (2 != n)
		__builtin_abort();

	if (2 != sizeof(*bar3()))
		__builtin_abort();
}

