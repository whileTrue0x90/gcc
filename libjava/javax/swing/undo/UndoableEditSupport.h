
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_undo_UndoableEditSupport__
#define __javax_swing_undo_UndoableEditSupport__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace javax
  {
    namespace swing
    {
      namespace event
      {
          class UndoableEditListener;
      }
      namespace undo
      {
          class CompoundEdit;
          class UndoableEdit;
          class UndoableEditSupport;
      }
    }
  }
}

class javax::swing::undo::UndoableEditSupport : public ::java::lang::Object
{

public:
  UndoableEditSupport();
  UndoableEditSupport(::java::lang::Object *);
  virtual ::java::lang::String * toString();
  virtual void addUndoableEditListener(::javax::swing::event::UndoableEditListener *);
  virtual void removeUndoableEditListener(::javax::swing::event::UndoableEditListener *);
  virtual JArray< ::javax::swing::event::UndoableEditListener * > * getUndoableEditListeners();
public: // actually protected
  virtual void _postEdit(::javax::swing::undo::UndoableEdit *);
public:
  virtual void postEdit(::javax::swing::undo::UndoableEdit *);
  virtual jint getUpdateLevel();
  virtual void beginUpdate();
public: // actually protected
  virtual ::javax::swing::undo::CompoundEdit * createCompoundEdit();
public:
  virtual void endUpdate();
public: // actually protected
  jint __attribute__((aligned(__alignof__( ::java::lang::Object)))) updateLevel;
  ::javax::swing::undo::CompoundEdit * compoundEdit;
  ::java::util::Vector * listeners;
  ::java::lang::Object * realSource;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_undo_UndoableEditSupport__
