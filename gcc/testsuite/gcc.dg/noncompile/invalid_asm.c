/* { dg-options "-ffat-lto-objects" } */
asm_invalid_register_name()
{
  asm("":::"this_is_an_invalid_register_name");	/* { dg-error "unknown register" } */
}
