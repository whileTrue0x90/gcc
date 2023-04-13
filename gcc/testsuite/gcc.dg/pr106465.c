/* PR c/106465
 * { dg-do run }
 * { dg-options "-std=gnu99" }
 * */

int main()
{
	int n = 3;
	
	void g1(int m, struct { char p[++m]; }* b)	/* { dg-warning "anonymous struct" } */
	{
		if (3 != m)
			__builtin_abort();

		if (3 != sizeof(b->p))
			__builtin_abort();
	}

	void g2(struct { char p[++n]; }* b)	/* { dg-warning "anonymous struct" } */
	{ 
		if (4 != n)
			__builtin_abort();

		if (4 != sizeof(b->p))
			__builtin_abort();
	}

	void g2b(struct { char (*p)[++n]; }* b)	/* { dg-warning "anonymous struct" } */
	{ 
		if (5 != n)
			__builtin_abort();

		if (5 != sizeof(*b->p))
			__builtin_abort();
	}

	if (3 != n)
		__builtin_abort();

	g1(2, (void*)0);
	g2((void*)0);
	g2b((void*)0);
	n--;

	if (4 != n)
		__builtin_abort();

	struct foo { char (*p)[++n]; } x;

	if (5 != n)
		__builtin_abort();

	struct bar { char (*p)[++n]; };

	if (6 != n)
		__builtin_abort();

	auto struct z { char (*p)[++n]; } g3(void);

	if (7 != n)
		__builtin_abort();

	struct z g3(void) { };

	if (7 != n)
		__builtin_abort();

	struct { char (*p)[++n]; } g4(void) { };

	if (8 != n)
		__builtin_abort();

	__auto_type u = g3();

	if (8 != n)
		__builtin_abort();

	if (5 != sizeof *x.p)
		__builtin_abort();

	if (7 != sizeof *u.p)
		__builtin_abort();

	return 0;
}

