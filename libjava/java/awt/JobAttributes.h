
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_awt_JobAttributes__
#define __java_awt_JobAttributes__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class JobAttributes;
        class JobAttributes$DefaultSelectionType;
        class JobAttributes$DestinationType;
        class JobAttributes$DialogType;
        class JobAttributes$MultipleDocumentHandlingType;
        class JobAttributes$SidesType;
    }
  }
}

class java::awt::JobAttributes : public ::java::lang::Object
{

public:
  JobAttributes();
  JobAttributes(::java::awt::JobAttributes *);
  JobAttributes(jint, ::java::awt::JobAttributes$DefaultSelectionType *, ::java::awt::JobAttributes$DestinationType *, ::java::awt::JobAttributes$DialogType *, ::java::lang::String *, jint, jint, ::java::awt::JobAttributes$MultipleDocumentHandlingType *, JArray< JArray< jint > * > *, ::java::lang::String *, ::java::awt::JobAttributes$SidesType *);
  ::java::lang::Object * clone();
  void set(::java::awt::JobAttributes *);
  jint getCopies();
  void setCopies(jint);
  void setCopiesToDefault();
  ::java::awt::JobAttributes$DefaultSelectionType * getDefaultSelection();
  void setDefaultSelection(::java::awt::JobAttributes$DefaultSelectionType *);
  ::java::awt::JobAttributes$DestinationType * getDestination();
  void setDestination(::java::awt::JobAttributes$DestinationType *);
  ::java::awt::JobAttributes$DialogType * getDialog();
  void setDialog(::java::awt::JobAttributes$DialogType *);
  ::java::lang::String * getFileName();
  void setFileName(::java::lang::String *);
  jint getFromPage();
  void setFromPage(jint);
  jint getMaxPage();
  void setMaxPage(jint);
  jint getMinPage();
  void setMinPage(jint);
  ::java::awt::JobAttributes$MultipleDocumentHandlingType * getMultipleDocumentHandling();
  void setMultipleDocumentHandling(::java::awt::JobAttributes$MultipleDocumentHandlingType *);
  void setMultipleDocumentHandlingToDefault();
  JArray< JArray< jint > * > * getPageRanges();
  void setPageRanges(JArray< JArray< jint > * > *);
  ::java::lang::String * getPrinter();
  void setPrinter(::java::lang::String *);
  ::java::awt::JobAttributes$SidesType * getSides();
  void setSides(::java::awt::JobAttributes$SidesType *);
  void setSidesToDefault();
  jint getToPage();
  void setToPage(jint);
  jboolean equals(::java::lang::Object *);
  jint hashCode();
  ::java::lang::String * toString();
private:
  jint __attribute__((aligned(__alignof__( ::java::lang::Object)))) copies;
  ::java::awt::JobAttributes$DefaultSelectionType * selection;
  ::java::awt::JobAttributes$DestinationType * destination;
  ::java::awt::JobAttributes$DialogType * dialog;
  ::java::lang::String * filename;
  jint maxPage;
  jint minPage;
  ::java::awt::JobAttributes$MultipleDocumentHandlingType * multiple;
  JArray< JArray< jint > * > * pageRanges;
  jint fromPage;
  jint toPage;
  ::java::lang::String * printer;
  ::java::awt::JobAttributes$SidesType * sides;
public:
  static ::java::lang::Class class$;
};

#endif // __java_awt_JobAttributes__
