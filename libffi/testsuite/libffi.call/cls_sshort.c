/* Area:	closure_call
   Purpose:	Check return value sshort.
   Limitations:	none.
   PR:		none.
   Originator:	<andreast@gcc.gnu.org> 20031108	 */

/* { dg-do run { xfail mips*-*-* arm*-*-* strongarm*-*-* xscale*-*-* } } */
#include "ffitest.h"

static void cls_ret_sshort_fn(ffi_cif* cif,void* resp,void** args,
			     void* userdata)
{
  *(ffi_arg*)resp = *(signed short *)args[0];
  printf("%d: %d\n",*(signed short *)args[0],
	 *(ffi_arg*)resp);
}
typedef signed short (*cls_ret_sshort)(signed short);

int main (void)
{
  ffi_cif cif;
  static ffi_closure cl;
  ffi_closure *pcl = &cl;
  ffi_type * cl_arg_types[2];
  signed short res;

  cl_arg_types[0] = &ffi_type_sint16;
  cl_arg_types[1] = NULL;

  /* Initialize the cif */
  CHECK(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1,
		     &ffi_type_sint16, cl_arg_types) == FFI_OK);

  CHECK(ffi_prep_closure(pcl, &cif, cls_ret_sshort_fn, NULL)  == FFI_OK);

  res = (*((cls_ret_sshort)pcl))(255);
  /* { dg-output "255: 255" } */
  printf("res: %d\n",res);
  /* { dg-output "\nres: 255" } */

  exit(0);
}
