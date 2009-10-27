
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_awt_dnd_peer_DropTargetContextPeer__
#define __java_awt_dnd_peer_DropTargetContextPeer__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
      namespace datatransfer
      {
          class DataFlavor;
          class Transferable;
      }
      namespace dnd
      {
          class DropTarget;
        namespace peer
        {
            class DropTargetContextPeer;
        }
      }
    }
  }
}

class java::awt::dnd::peer::DropTargetContextPeer : public ::java::lang::Object
{

public:
  virtual void setTargetActions(jint) = 0;
  virtual jint getTargetActions() = 0;
  virtual ::java::awt::dnd::DropTarget * getDropTarget() = 0;
  virtual JArray< ::java::awt::datatransfer::DataFlavor * > * getTransferDataFlavors() = 0;
  virtual ::java::awt::datatransfer::Transferable * getTransferable() = 0;
  virtual jboolean isTransferableJVMLocal() = 0;
  virtual void acceptDrag(jint) = 0;
  virtual void rejectDrag() = 0;
  virtual void acceptDrop(jint) = 0;
  virtual void rejectDrop() = 0;
  virtual void dropComplete(jboolean) = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __java_awt_dnd_peer_DropTargetContextPeer__
