
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_sound_midi_MidiDevice$Info__
#define __javax_sound_midi_MidiDevice$Info__

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
          class MidiDevice$Info;
      }
    }
  }
}

class javax::sound::midi::MidiDevice$Info : public ::java::lang::Object
{

public: // actually protected
  MidiDevice$Info(::java::lang::String *, ::java::lang::String *, ::java::lang::String *, ::java::lang::String *);
public:
  virtual jboolean equals(::java::lang::Object *);
  virtual jint hashCode();
  virtual ::java::lang::String * getName();
  virtual ::java::lang::String * getVendor();
  virtual ::java::lang::String * getDescription();
  virtual ::java::lang::String * getVersion();
  virtual ::java::lang::String * toString();
private:
  ::java::lang::String * __attribute__((aligned(__alignof__( ::java::lang::Object)))) name;
  ::java::lang::String * vendor;
  ::java::lang::String * description;
  ::java::lang::String * version;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_sound_midi_MidiDevice$Info__
