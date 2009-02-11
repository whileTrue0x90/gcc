
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_crypto_pad_TBC__
#define __gnu_javax_crypto_pad_TBC__

#pragma interface

#include <gnu/javax/crypto/pad/BasePad.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace crypto
      {
        namespace pad
        {
            class TBC;
        }
      }
    }
  }
}

class gnu::javax::crypto::pad::TBC : public ::gnu::javax::crypto::pad::BasePad
{

public: // actually package-private
  TBC();
public:
  void setup();
  JArray< jbyte > * pad(JArray< jbyte > *, jint, jint);
  jint unpad(JArray< jbyte > *, jint, jint);
private:
  static ::java::util::logging::Logger * log;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_crypto_pad_TBC__
