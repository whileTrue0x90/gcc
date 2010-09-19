#include <objc/objc.h>
#include <objc/Object.h>

/* Test throwing a nil exception.  A 'nil' exception can only be
 * caugth by a generic exception handler.
 */

int main (void)
{
  int exception_catched = 0;
  int finally_called = 0;

  @try
    {
      @throw nil;
    }
  @catch (Object *exc)
    {
      abort ();
    }
  @catch (id exc)
    {
      exception_catched = 1;
    }
  @finally
    {
      finally_called = 1;
    }


  if (exception_catched != 1
      || finally_called != 1)
    {
      abort ();
    }

  return 0;
}
