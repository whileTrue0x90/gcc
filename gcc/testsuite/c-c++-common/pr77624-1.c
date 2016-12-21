/* PR middle-end/77624 */
/* { dg-do compile } */

int
foo (int a)
{
  return __atomic_is_lock_free (2, a);		/* { dg-warning "pointer from integer" "" { target c } } */
}						/* { dg-error "invalid conversion" "" { target c++ } 7 } */

int
bar (int a)
{
  return __atomic_always_lock_free (2, a);	/* { dg-warning "pointer from integer" "" { target c } } */
}						/* { dg-error "invalid conversion" "" { target c++ } 13 } */
