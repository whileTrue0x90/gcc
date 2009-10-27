
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_plaf_basic_BasicDirectoryModel__
#define __javax_swing_plaf_basic_BasicDirectoryModel__

#pragma interface

#include <javax/swing/AbstractListModel.h>
extern "Java"
{
  namespace java
  {
    namespace beans
    {
        class PropertyChangeEvent;
    }
  }
  namespace javax
  {
    namespace swing
    {
        class JFileChooser;
      namespace event
      {
          class ListDataEvent;
      }
      namespace plaf
      {
        namespace basic
        {
            class BasicDirectoryModel;
            class BasicDirectoryModel$DirectoryLoadThread;
        }
      }
    }
  }
}

class javax::swing::plaf::basic::BasicDirectoryModel : public ::javax::swing::AbstractListModel
{

public:
  BasicDirectoryModel(::javax::swing::JFileChooser *);
  virtual jboolean contains(::java::lang::Object *);
  virtual void fireContentsChanged();
  virtual ::java::util::Vector * getDirectories();
  virtual ::java::lang::Object * getElementAt(jint);
  virtual ::java::util::Vector * getFiles();
  virtual jint getSize();
  virtual jint indexOf(::java::lang::Object *);
  virtual void intervalAdded(::javax::swing::event::ListDataEvent *);
  virtual void intervalRemoved(::javax::swing::event::ListDataEvent *);
  virtual void invalidateFileCache();
public: // actually protected
  virtual jboolean lt(::java::io::File *, ::java::io::File *);
public:
  virtual void propertyChange(::java::beans::PropertyChangeEvent *);
  virtual jboolean renameFile(::java::io::File *, ::java::io::File *);
public: // actually protected
  virtual void sort(::java::util::Vector *);
public:
  virtual void validateFileCache();
public: // actually package-private
  static ::java::util::Vector * access$0(::javax::swing::plaf::basic::BasicDirectoryModel *);
  static void access$1(::javax::swing::plaf::basic::BasicDirectoryModel *, ::java::util::Vector *);
  static void access$2(::javax::swing::plaf::basic::BasicDirectoryModel *, ::java::util::Vector *);
  static void access$3(::javax::swing::plaf::basic::BasicDirectoryModel *, ::java::lang::Object *, jint, jint);
  static void access$4(::javax::swing::plaf::basic::BasicDirectoryModel *, ::java::lang::Object *, jint, jint);
  static ::javax::swing::JFileChooser * access$5(::javax::swing::plaf::basic::BasicDirectoryModel *);
private:
  ::java::util::Vector * __attribute__((aligned(__alignof__( ::javax::swing::AbstractListModel)))) contents;
  ::java::util::Vector * directories;
  ::java::util::Vector * files;
  jint listingMode;
  ::javax::swing::JFileChooser * filechooser;
  ::javax::swing::plaf::basic::BasicDirectoryModel$DirectoryLoadThread * loadThread;
  ::java::util::Comparator * comparator;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_plaf_basic_BasicDirectoryModel__
