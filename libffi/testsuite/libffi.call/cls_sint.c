/* Area:	closure_call
   Purpose:	Check return value sint32.
   Limitations:	none.
   PR:		none.
   Originator:	<andreast@gcc.gnu.org> 20031108	 */

/* { dg-do run { xfail mips*-*-* arm*-*-* strongarm*-*-* xscale*-*-* } } */
#include "ffitest.h"

static void cls_ret_sint_fn(ffi_cif* cif,void* resp,void** args,
			     void* userdata)
{
  *(ffi_arg*)resp = *(signed int *)args[0];
  printf("%d: %d\n",*(signed int *)args[0],
	 *(ffi_arg*)resp);
}
typedef signed int (*cls_ret_sint)(signed int);

int main (void)
{
  ffi_cif cif;
  static ffi_closure cl;
  ffi_closure *pcl = &cl;
  ffi_type * cl_arg_types[2];
  signed int res;

  cl_arg_types[0] = &ffi_type_sint32;
  cl_arg_types[1] = NULL;

  /* Initialize the cif */
  CHECK(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1,
		     &ffi_type_sint32, cl_arg_types) == FFI_OK);

  CHECK(ffi_prep_closure(pcl, &cif, cls_ret_sint_fn, NULL)  == FFI_OK);

  res = (*((cls_ret_sint)pcl))(65534);
  /* { dg-output "65534: 65534" } */
  printf("res: %d\n",res);
  /* { dg-output "\nres: 65534" } */

  exit(0);
}
