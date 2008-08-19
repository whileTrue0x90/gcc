// { dg-do "compile" }
// { dg-options "-std=c++0x"}

#include <cassert>

int main() {
  int i = 1;

  [] (int& i) -> void {
    [&] () -> void {
      i = 2;
    } ();
  } (i);

  assert(i == 2);

  /*
  [&] () -> void {
    [&i] () -> void {
      i = 3;
    } ();
  } ();

  assert(i == 3);

  [&] () -> void {
    [&] () -> void {
      i = 4;
    } ();
  } ();

  assert(i == 4);
  */

  return 0;
}

