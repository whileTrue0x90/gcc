
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_sound_midi_file_MidiFileWriter__
#define __gnu_javax_sound_midi_file_MidiFileWriter__

#pragma interface

#include <javax/sound/midi/spi/MidiFileWriter.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace sound
      {
        namespace midi
        {
          namespace file
          {
              class MidiDataOutputStream;
              class MidiFileWriter;
          }
        }
      }
    }
  }
  namespace javax
  {
    namespace sound
    {
      namespace midi
      {
          class Sequence;
          class Track;
      }
    }
  }
}

class gnu::javax::sound::midi::file::MidiFileWriter : public ::javax::sound::midi::spi::MidiFileWriter
{

public:
  MidiFileWriter();
  virtual JArray< jint > * getMidiFileTypes();
  virtual JArray< jint > * getMidiFileTypes(::javax::sound::midi::Sequence *);
  virtual jint write(::javax::sound::midi::Sequence *, jint, ::java::io::OutputStream *);
private:
  jint computeTrackLength(::javax::sound::midi::Track *, ::gnu::javax::sound::midi::file::MidiDataOutputStream *);
  jint writeTrack(::javax::sound::midi::Track *, ::gnu::javax::sound::midi::file::MidiDataOutputStream *);
public:
  virtual jint write(::javax::sound::midi::Sequence *, jint, ::java::io::File *);
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_sound_midi_file_MidiFileWriter__
