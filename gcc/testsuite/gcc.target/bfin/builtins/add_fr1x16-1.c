extern void abort (void);

typedef short fract16;

int main ()
{
  fract16 t1;

  t1 = __builtin_bfin_add_fr1x16 (0x3000, 0x2000);
  if (t1 != 0x5000)
    abort ();
  return 0;
}

