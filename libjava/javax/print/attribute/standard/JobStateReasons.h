
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_print_attribute_standard_JobStateReasons__
#define __javax_print_attribute_standard_JobStateReasons__

#pragma interface

#include <java/util/HashSet.h>
extern "Java"
{
  namespace javax
  {
    namespace print
    {
      namespace attribute
      {
        namespace standard
        {
            class JobStateReason;
            class JobStateReasons;
        }
      }
    }
  }
}

class javax::print::attribute::standard::JobStateReasons : public ::java::util::HashSet
{

public:
  JobStateReasons();
  JobStateReasons(jint, jfloat);
  JobStateReasons(jint);
  JobStateReasons(::java::util::Collection *);
  jboolean JobStateReasons$add(::javax::print::attribute::standard::JobStateReason *);
  ::java::lang::Class * getCategory();
  ::java::lang::String * getName();
  jboolean add(::java::lang::Object *);
private:
  static const jlong serialVersionUID = 8849088261264331812LL;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_print_attribute_standard_JobStateReasons__
