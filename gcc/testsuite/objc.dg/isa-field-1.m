/* Ensure there are no bizarre difficulties with accessing the 'isa' field of objects.  */
/* { dg-do compile } */

#include "../objc-obj-c++-shared/Object1.h"

@interface Object (Test)
- (Class) test1: (id)object;
@end

@interface Derived: Object
- (Class) test2: (id)object;
@end

@implementation Object (Test)

Class test1(id object) {
#ifdef __NEXT_RUNTIME__
    Class cls = object->isa;
#else
    Class cls = object->class_pointer;
#endif
    return cls;
}
- (Class) test1: (id)object {
#ifdef __NEXT_RUNTIME__
    Class cls = object->isa;
#else
    Class cls = object->class_pointer;
#endif
    return cls;
}

@end

@implementation Derived

Class test2(id object) {
#ifdef __NEXT_RUNTIME__
    Class cls = object->isa;
#else
    Class cls = object->class_pointer;
#endif
    return cls;
}
- (Class) test2: (id)object {
#ifdef __NEXT_RUNTIME__
    Class cls = object->isa;
#else
    Class cls = object->class_pointer;
#endif
    return cls;
}

@end

Class test3(id object) {
#ifdef __NEXT_RUNTIME__
    Class cls = object->isa;
#else
    Class cls = object->class_pointer;
#endif
    return cls;
}
