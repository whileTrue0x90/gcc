
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_activation_viewers_ImageViewer__
#define __gnu_javax_activation_viewers_ImageViewer__

#pragma interface

#include <java/awt/Component.h>
extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace activation
      {
        namespace viewers
        {
            class ImageViewer;
        }
      }
    }
  }
  namespace java
  {
    namespace awt
    {
        class Dimension;
        class Graphics;
        class Image;
    }
  }
  namespace javax
  {
    namespace activation
    {
        class DataHandler;
    }
  }
}

class gnu::javax::activation::viewers::ImageViewer : public ::java::awt::Component
{

public:
  ImageViewer();
  virtual ::java::awt::Dimension * getPreferredSize();
  virtual void setCommandContext(::java::lang::String *, ::javax::activation::DataHandler *);
  virtual jboolean imageUpdate(::java::awt::Image *, jint, jint, jint, jint, jint);
  virtual void paint(::java::awt::Graphics *);
private:
  ::java::awt::Image * __attribute__((aligned(__alignof__( ::java::awt::Component)))) image;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_activation_viewers_ImageViewer__
