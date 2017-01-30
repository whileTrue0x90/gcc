// { dg-do run }
// { dg-shouldfail "asan" }
// { dg-additional-options "-O2 -fdump-tree-asan1" }

int
main (int argc, char **argv)
{
  int *ptr = 0;

  {
    int a;
    ptr = &a;
    *ptr = 12345;
  }

  return *ptr;
}

// { dg-final { scan-tree-dump-times "= ASAN_POISON \\(\\)" 1 "asan1" } }
// { dg-output "ERROR: AddressSanitizer: stack-use-after-scope on address.*(\n|\r\n|\r)" }
// { dg-output "READ of size .*" }
// { dg-output ".*'a' <== Memory access at offset \[0-9\]* is inside this variable.*" }
