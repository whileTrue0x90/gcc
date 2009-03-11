/* This tests passing of structs. */

#include "defines.h"
#include "args.h"

struct IntegerRegisters iregs;
struct FloatRegisters fregs;
unsigned int num_iregs, num_fregs;

struct int_struct
{
  int i;
};

struct long_struct
{
  long l;
};

struct long2_struct
{
  long l1, l2;
};

struct long3_struct
{
  long l1, l2, l3;
};


/* Check that the struct is passed as the individual members in iregs.  */
void
check_struct_passing1 (struct int_struct is ATTRIBUTE_UNUSED)
{
  check_int_arguments;
}

void
check_struct_passing2 (struct long_struct ls ATTRIBUTE_UNUSED)
{
  check_int_arguments;
}

void
check_struct_passing3 (struct long2_struct ls ATTRIBUTE_UNUSED)
{
  check_int_arguments;
}

void
check_struct_passing4 (struct long3_struct ls ATTRIBUTE_UNUSED)
{
  /* Check the passing on the stack by comparing the address of the
     stack elements to the expected place on the stack.  */
  assert ((unsigned long)&ls.l1 == rsp+8);
  assert ((unsigned long)&ls.l2 == rsp+16);
  assert ((unsigned long)&ls.l3 == rsp+24);
}

#ifdef CHECK_M64_M128
struct m128_struct
{
  __m128 x;
};

struct m128_2_struct
{
  __m128 x1, x2;
};

/* Check that the struct is passed as the individual members in fregs.  */
void
check_struct_passing5 (struct m128_struct ms1 ATTRIBUTE_UNUSED,
		       struct m128_struct ms2 ATTRIBUTE_UNUSED,
		       struct m128_struct ms3 ATTRIBUTE_UNUSED,
		       struct m128_struct ms4 ATTRIBUTE_UNUSED,
		       struct m128_struct ms5 ATTRIBUTE_UNUSED,
		       struct m128_struct ms6 ATTRIBUTE_UNUSED,
		       struct m128_struct ms7 ATTRIBUTE_UNUSED,
		       struct m128_struct ms8 ATTRIBUTE_UNUSED)
{
  check_m128_arguments;
}

void
check_struct_passing6 (struct m128_2_struct ms ATTRIBUTE_UNUSED)
{
  /* Check the passing on the stack by comparing the address of the
     stack elements to the expected place on the stack.  */
  assert ((unsigned long)&ms.x1 == rsp+8);
  assert ((unsigned long)&ms.x2 == rsp+24);
}
#endif

int
main (void)
{
  struct int_struct is = { 48 };
  struct long_struct ls = { 49 };
#ifdef CHECK_LARGER_STRUCTS
  struct long2_struct l2s = { 50, 51 };
  struct long3_struct l3s = { 52, 53, 54 };
#endif
#ifdef CHECK_M64_M128
  struct m128_struct m128s[8];
  struct m128_2_struct m128_2s = { 
      { 48.394, 39.3, -397.9, 3484.9 },
      { -8.394, -93.3, 7.9, 84.94 }
  };
  int i;
#endif

  clear_struct_registers;
  iregs.I0 = is.i;
  num_iregs = 1;
  clear_int_hardware_registers;
  WRAP_CALL (check_struct_passing1)(is);

  clear_struct_registers;
  iregs.I0 = ls.l;
  num_iregs = 1;
  clear_int_hardware_registers;
  WRAP_CALL (check_struct_passing2)(ls);

#ifdef CHECK_LARGER_STRUCTS
  clear_struct_registers;
  iregs.I0 = l2s.l1;
  iregs.I1 = l2s.l2;
  num_iregs = 2;
  clear_int_hardware_registers;
  WRAP_CALL (check_struct_passing3)(l2s);
  WRAP_CALL (check_struct_passing4)(l3s);
#endif

#ifdef CHECK_M64_M128
  clear_struct_registers;
  for (i = 0; i < 8; i++)
    {
      m128s[i].x = (__m128){32+i, 0, i, 0};
      fregs.xmm0._m128[i] = m128s[i].x;
    }
  num_fregs = 8;
  clear_float_hardware_registers;
  WRAP_CALL (check_struct_passing5)(m128s[0], m128s[1], m128s[2], m128s[3],
				    m128s[4], m128s[5], m128s[6], m128s[7]);
  WRAP_CALL (check_struct_passing6)(m128_2s);
#endif

  return 0;
}
