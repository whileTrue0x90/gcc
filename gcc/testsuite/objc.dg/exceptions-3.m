/* Contributed by Nicola Pero <nicola.pero@meta-innovation.com>, November 2010.  */
/* { dg-options "-fobjc-exceptions" } */
/* { dg-do compile } */

/* Test that the compiler is checking the argument of @catch(), and
   produce errors when invalid types are used.  */

#include <objc/objc.h>

@interface MyObject
{
  Class isa;
}
@end

@implementation MyObject
@end

@protocol MyProtocol;

typedef MyObject MyObjectTypedef;
typedef MyObject *MyObjectPtrTypedef;
typedef int intTypedef;

int test (id object)
{
  int dummy = 0;

  @try { @throw object; }
  @catch (int x)          /* { dg-error "@catch parameter is not a known Objective-C class type" } */
    {
      dummy++;
    }

  @try { @throw object; }
  @catch (intTypedef x)   /* { dg-error "@catch parameter is not a known Objective-C class type" } */
    {
      dummy++;
    }

  @try { @throw object; }
  @catch (int *x)         /* { dg-error "@catch parameter is not a known Objective-C class type" } */
    {
      dummy++;
    }  

  @try { @throw object; }
  @catch (id x)           /* Ok */
    {
      dummy++;
    }

  @try { @throw object; }
  @catch (id <MyProtocol> x) /* { dg-error "@catch parameter can not be protocol-qualified" } */
    {
      dummy++;
    }

  @try { @throw object; }
  @catch (MyObject *x)    /* Ok */
    {
      dummy++;
    }

  @try { @throw object; }
  @catch (MyObject <MyProtocol> *x)  /* { dg-error "@catch parameter can not be protocol-qualified" } */
    {
      dummy++;
    }

  @try { @throw object; }
  @catch (MyObject x)     /* { dg-error "@catch parameter is not a known Objective-C class type" } */
    {                     /* { dg-error "conversion to non-scalar type requested" "" { target *-*-* } 72 } */
      dummy++;
    }

  @try { @throw object; }
  @catch (static MyObject *x) /* { dg-error "storage class specified for" } */
    {
      dummy++;
    }

  @try { @throw object; }
  @catch (MyObjectTypedef *x) /* Ok */
    {
      dummy++;
    }

  @try { @throw object; }
  @catch (MyObjectTypedef <MyProtocol> *x) /* { dg-error "@catch parameter can not be protocol-qualified" } */
    {
      dummy++;
    }

  @try { @throw object; }
  @catch (MyObjectPtrTypedef x) /* Ok */
    {
      dummy++;
    }

  @try { @throw object; }
  @catch (Class x)   /* { dg-error "@catch parameter is not a known Objective-C class type" } */
    {
      dummy++;
    }

  @try { @throw object; }
  @catch (...)            /* Ok */
    {
      dummy++;
    }

  return dummy;
}
