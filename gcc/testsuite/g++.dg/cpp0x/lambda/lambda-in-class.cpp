// { dg-do "compile" }
// { dg-options "-std=c++0x"}

#include <cassert>

class C {
  private:
    int m_i;

  public:
    C() : m_i(-1) {
      [this] () -> void { m_i = 0; } ();
      assert(m_i == 0);
      [this] () -> void { this->m_i = 1; } ();
      assert(m_i == 1);
      [&] () -> void { m_i = 2; } ();
      assert(m_i == 2);
      [&] () -> void { this->m_i = 3; } ();
      assert(m_i == 3);
      [=] () -> void { m_i = 4; } (); // copies 'this' or --copies-m_i--?
      assert(m_i == 4);
      [=] () -> void { this->m_i = 5; } ();
      assert(m_i == 5);
    }

};

int main() {
  C c;

  //[this] () -> void {} (); // { dg-error: "cannot capture `this' outside of class method" }

  return 0;
}

