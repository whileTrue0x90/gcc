
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_sound_midi_Transmitter__
#define __javax_sound_midi_Transmitter__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace javax
  {
    namespace sound
    {
      namespace midi
      {
          class Receiver;
          class Transmitter;
      }
    }
  }
}

class javax::sound::midi::Transmitter : public ::java::lang::Object
{

public:
  virtual void setReceiver(::javax::sound::midi::Receiver *) = 0;
  virtual ::javax::sound::midi::Receiver * getReceiver() = 0;
  virtual void close() = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __javax_sound_midi_Transmitter__
