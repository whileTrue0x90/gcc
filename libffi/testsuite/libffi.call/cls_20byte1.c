/* Area:	ffi_call, closure_call
   Purpose:	Check structure passing with different structure size.
		Depending on the ABI. Check overlapping.
   Limitations:	none.
   PR:		none.
   Originator:	<andreast@gcc.gnu.org> 20030828	 */

/* { dg-do run { xfail mips*-*-* arm*-*-* strongarm*-*-* xscale*-*-* } } */
#include "ffitest.h"

typedef struct cls_struct_20byte {
  int a;
  double b;
  double c;
} cls_struct_20byte;

cls_struct_20byte cls_struct_20byte_fn(struct cls_struct_20byte a1,
			    struct cls_struct_20byte a2)
{
  struct cls_struct_20byte result;

  result.a = a1.a + a2.a;
  result.b = a1.b + a2.b;
  result.c = a1.c + a2.c;

  printf("%d %g %g %d %g %g: %d %g %g\n", a1.a, a1.b, a1.c, a2.a, a2.b, a2.c,
	 result.a, result.b, result.c);
  return result;
}

static void
cls_struct_20byte_gn(ffi_cif* cif, void* resp, void** args, void* userdata)
{
  struct cls_struct_20byte a1, a2;

  a1 = *(struct cls_struct_20byte*)(args[0]);
  a2 = *(struct cls_struct_20byte*)(args[1]);

  *(cls_struct_20byte*)resp = cls_struct_20byte_fn(a1, a2);
}

int main (void)
{
  ffi_cif cif;
  static ffi_closure cl;
  ffi_closure *pcl = &cl;
  void* args_dbl[3];
  ffi_type* cls_struct_fields[4];
  ffi_type cls_struct_type;
  ffi_type* dbl_arg_types[3];

  cls_struct_type.size = 0;
  cls_struct_type.alignment = 0;
  cls_struct_type.type = FFI_TYPE_STRUCT;
  cls_struct_type.elements = cls_struct_fields;

  struct cls_struct_20byte g_dbl = { 1, 2.0, 3.0 };
  struct cls_struct_20byte f_dbl = { 4, 5.0, 7.0 };
  struct cls_struct_20byte res_dbl;

  cls_struct_fields[0] = &ffi_type_uint32;
  cls_struct_fields[1] = &ffi_type_double;
  cls_struct_fields[2] = &ffi_type_double;
  cls_struct_fields[3] = NULL;

  dbl_arg_types[0] = &cls_struct_type;
  dbl_arg_types[1] = &cls_struct_type;
  dbl_arg_types[2] = NULL;

  CHECK(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 2, &cls_struct_type,
		     dbl_arg_types) == FFI_OK);

  args_dbl[0] = &g_dbl;
  args_dbl[1] = &f_dbl;
  args_dbl[2] = NULL;

  ffi_call(&cif, FFI_FN(cls_struct_20byte_fn), &res_dbl, args_dbl);
  /* { dg-output "1 2 3 4 5 7: 5 7 10" } */
  printf("res: %d %g %g\n", res_dbl.a, res_dbl.b, res_dbl.c);
  /* { dg-output "\nres: 5 7 10" } */

  CHECK(ffi_prep_closure(pcl, &cif, cls_struct_20byte_gn, NULL) == FFI_OK);

  res_dbl = ((cls_struct_20byte(*)(cls_struct_20byte, cls_struct_20byte))(pcl))(g_dbl, f_dbl);
  /* { dg-output "\n1 2 3 4 5 7: 5 7 10" } */
  printf("res: %d %g %g\n", res_dbl.a, res_dbl.b, res_dbl.c);
  /* { dg-output "\nres: 5 7 10" } */

  exit(0);
}
