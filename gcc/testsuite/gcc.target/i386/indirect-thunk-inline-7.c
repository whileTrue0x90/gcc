/* { dg-do compile } */
/* { dg-options "-O2 -mno-indirect-branch-register -mfunction-return=keep -mindirect-branch=thunk-inline -fno-pic" } */

void func0 (void);
void func1 (void);
void func2 (void);
void func3 (void);
void func4 (void);
void func4 (void);
void func5 (void);

void
bar (int i)
{
  switch (i)
    {
    default:
      func0 ();
      break;
    case 1:
      func1 ();
      break;
    case 2:
      func2 ();
      break;
    case 3:
      func3 ();
      break;
    case 4:
      func4 ();
      break;
    case 5:
      func5 ();
      break;
    }
}

/* { dg-final { scan-assembler "push(?:l|q)\[ \t\]*\.L\[0-9\]+\\(,%" { target { { ! x32 } && *-*-linux* } } } } */
/* { dg-final { scan-assembler-not "pushq\[ \t\]%rax" { target x32 } } } */
/* { dg-final { scan-assembler "jmp\[ \t\]*\.LIND" } } */
/* { dg-final { scan-assembler "call\[ \t\]*\.LIND" } } */
/* { dg-final { scan-assembler {\tpause} } } */
/* { dg-final { scan-assembler {\tlfence} } } */
/* { dg-final { scan-assembler-not "__x86_indirect_thunk" } } */
