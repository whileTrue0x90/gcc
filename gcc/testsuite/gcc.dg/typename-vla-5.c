/* { dg-do run } 
 * { dg-options "-std=gnu99" }
 * */

int
main (void)
{
	int a = 1;
	if (sizeof (*(++a, (struct { char (*x)[a]; }){ 0 }).x) != 2)
 		__builtin_abort ();
}


