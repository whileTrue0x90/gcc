/* GNU Objective-C Runtime API - Modern API
   Copyright (C) 2010 Free Software Foundation, Inc.
   Contributed by Nicola Pero <nicola.pero@meta-innovation.com>

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

GCC is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

#ifndef __objc_runtime_INCLUDE_GNU
#define __objc_runtime_INCLUDE_GNU

/*
  This file declares the "modern" GNU Objective-C Runtime API.
  Include this file to use it.

  This API is replacing the "traditional" GNU Objective-C Runtime API
  (declared in objc/objc-api.h) which is the one supported by older
  versions of the GNU Objective-C Runtime.  The "modern" API is very
  similar to the API used by the modern Apple/NeXT runtime.

  Because the two APIs have some conflicting definitions (in
  particular, Method and Category are defined differently) you should
  include either objc/objc-api.h (to use the traditional GNU
  Objective-C Runtime API) or objc/runtime.h (to use the modern GNU
  Objective-C Runtime API), but not both.
*/
#ifdef __objc_api_INCLUDE_GNU
# error You can not include both objc/objc-api.h and objc/runtime.h.  Include objc/objc-api.h for the traditional GNU Objective-C Runtime API and objc/runtime.h for the modern one.
#endif

/* TODO: This file is incomplete.  */

#include "objc.h"
#include "objc-decls.h"

/* An 'Ivar' represents an instance variable.  It holds information
   about the name, type and offset of the instance variable.  */
typedef struct objc_ivar *Ivar;

/* A 'Property' represents a property.  It holds information about the
   name of the property, and its attributes.

   Compatibility Note: the Apple/NeXT runtime defines this as
   objc_property_t, so we define it that way as well, but obviously
   Property is the right name.  */
typedef struct objc_property *Property;
typedef struct objc_property *objc_property_t;

/* A 'Method' represents a method.  It holds information about the
   name, types and the IMP of the method.  */
typedef struct objc_method *Method;

/* A 'Category' represents a category.  It holds information about the
   name of the category, the class it belongs to, and the methods,
   protocols and such like provided by the category.  */
typedef struct objc_category *Category;

/* 'Protocol' is defined in objc/objc.h (which is included by this
   file).  */

/* Method descriptor returned by introspective Object methods.  At the
   moment, this is really just the first part of the more complete
   objc_method structure used internally by the runtime.  (PS: In the
   GNU Objective-C Runtime, selectors already include a type, so an
   objc_method_description does not add much to a SEL.  But in other
   runtimes, that is not the case, which is why
   objc_method_description exists).  */
struct objc_method_description
{
  SEL name;      /* Selector (name and signature) */
  char *types;   /* Type encoding */
};

/* The following are used in encode strings to describe the type of
   Ivars and Methods.  */
#define _C_ID       '@'
#define _C_CLASS    '#'
#define _C_SEL      ':'
#define _C_CHR      'c'
#define _C_UCHR     'C'
#define _C_SHT      's'
#define _C_USHT     'S'
#define _C_INT      'i'
#define _C_UINT     'I'
#define _C_LNG      'l'
#define _C_ULNG     'L'
#define _C_LNG_LNG  'q'
#define _C_ULNG_LNG 'Q'
#define _C_FLT      'f'
#define _C_DBL      'd'
#define _C_LNG_DBL  'D'
#define _C_BFLD     'b'
#define _C_BOOL     'B'
#define _C_VOID     'v'
#define _C_UNDEF    '?'
#define _C_PTR      '^'
#define _C_CHARPTR  '*'
#define _C_ARY_B    '['
#define _C_ARY_E    ']'
#define _C_UNION_B  '('
#define _C_UNION_E  ')'
#define _C_STRUCT_B '{'
#define _C_STRUCT_E '}'
#define _C_VECTOR   '!'
#define _C_COMPLEX  'j'

/* _C_ATOM is never generated by the compiler.  You can treat it as
   equivalent to "*".  */
#define _C_ATOM     '%'

/* The following are used in encode strings to describe some
   qualifiers of method and ivar types.  */
#define _C_CONST	'r'
#define _C_IN		'n'
#define _C_INOUT	'N'
#define _C_OUT      	'o'
#define _C_BYCOPY	'O'
#define _C_BYREF	'R'
#define _C_ONEWAY	'V'
#define _C_GCINVISIBLE	'|'

/* The same when used as flags.  */
#define _F_CONST	0x01
#define _F_IN		0x01
#define _F_OUT		0x02
#define _F_INOUT	0x03
#define _F_BYCOPY	0x04
#define _F_BYREF	0x08
#define _F_ONEWAY	0x10
#define _F_GCINVISIBLE	0x20


/** Implementation: the following functions are defined inline.  */

/* Return the class of 'object', or Nil if the object is nil.  If
   'object' is a class, the meta class is returned; if 'object' is a
   meta class, the root meta class is returned (note that this is
   different from the traditional GNU Objective-C Runtime API function
   object_get_class(), which for a meta class would return the meta
   class itself).  This function is inline, so it is really fast and
   should be used instead of accessing object->class_pointer
   directly.  */
static inline Class
object_getClass (id object)
{
  if (object != nil)
    return object->class_pointer;
  else
    return Nil;
}


/** Implementation: the following functions are in selector.c.  */

/* Return the name of a given selector.  If 'selector' is NULL, return
   "<null selector>".  */
objc_EXPORT const char *sel_getName (SEL selector);

/* Return the type of a given selector.

   Compatibility Note: the Apple/NeXT runtime has untyped selectors,
   so it does not have this function, which is specific to the GNU
   Runtime.  */
objc_EXPORT const char *sel_getType (SEL selector);

/* This is the same as sel_registerName ().  Please use
   sel_registerName () instead.  */
objc_EXPORT SEL sel_getUid (const char *name);

/* Register a selector with a given name (but unspecified types).  If
   you know the types, it is better to call sel_registerTypedName().
   If a selector with this name already exists, it is returned.  */
objc_EXPORT SEL sel_registerName (const char *name);

/* Register a selector with a given name and types.  If a selector
   with this name and types already exists, it is returned.

   Compatibility Note: the Apple/NeXT runtime has untyped selectors,
   so it does not have this function, which is specific to the GNU
   Runtime.  */
objc_EXPORT SEL set_registerTypedName (const char *name, const char *type);

/* Return YES if first_selector is the same as second_selector, and NO
   if not.  */
objc_EXPORT BOOL sel_isEqual (SEL first_selector, SEL second_selector);


/** Implementation: the following functions are in objects.c.  */

/* Create an instance of class 'class_', adding extraBytes to the size
   of the returned object.  This method allocates the appropriate
   amount of memory for the instance, initializes it to zero, then
   calls all the C++ constructors on appropriate C++ instance
   variables of the instance (if any) (TODO: The C++ constructors bit
   is not implemented yet).  */
objc_EXPORT id class_createInstance (Class class_, size_t extraBytes);

/* Copy an object and return the copy.  extraBytes should be identical
   to the extraBytes parameter that was passed when creating the
   original object.  */
objc_EXPORT id object_copy (id object, size_t extraBytes);

/* Dispose of an object.  This method calls the appropriate C++
   destructors on appropriate C++ instance variables of the instance
   (if any) (TODO: This is not implemented yet), then frees the memory
   for the instance.  */
objc_EXPORT id object_dispose (id object);

/* Return the name of the class of 'object'.  If 'object' is 'nil',
   returns "Nil".  */
objc_EXPORT const char * object_getClassName (id object);

/* Change the class of object to be class_.  Return the previous class
   of object.  This is currently not really thread-safe.  */
objc_EXPORT Class object_setClass (id object, Class class_);


/** Implementation: the following functions are in ivars.c.  */

/* Return an instance variable given the class and the instance
   variable name.  This is an expensive function to call, so try to
   reuse the returned Ivar if you can.  */
objc_EXPORT Ivar class_getInstanceVariable (Class class_, const char *name);

/* Return a class variable given the class and the class variable
   name.  This is an expensive function to call, so try to reuse the
   returned Ivar if you can.  
   
   This function always returns NULL since class variables are
   currently unavailable in Objective-C.  */
objc_EXPORT Ivar class_getClassVariable (Class class_, const char *name);

/* If the object was created in class_createInstance() with some
   extraBytes, returns a pointer to them.  If it was not, then the
   returned pointer may make no sense.  */
objc_EXPORT void * object_getIndexedIvars (id object);

/* Get the value of an instance variable of type 'id'.  The function
   returns the instance variable.  To get the value of the instance
   variable, you should pass as 'returnValue' a pointer to an 'id';
   the value will be copied there.  Note that 'returnValue' is really
   a 'void *', not a 'void **'.  This function really works only with
   instance variables of type 'id'; for other types of instance
   variables, access directly the data at (char *)object +
   ivar_getOffset (ivar).  */
objc_EXPORT Ivar object_getInstanceVariable (id object, const char *name, void **returnValue);

/* Set the value of an instance variable.  The value to set is passed
   in 'newValue' (which really is an 'id', not a 'void *').  The
   function returns the instance variable.  This function really works
   only with instance variables of type 'id'; for other types of
   instance variables, access directly the data at (char *)object +
   ivar_getOffset (ivar).  */
objc_EXPORT Ivar object_setInstanceVariable (id object, const char *name, void *newValue);

/* Get the value of an instance variable of type 'id' of the object
   'object'.  This is faster than object_getInstanceVariable if you
   already have the instance variable because it avoids the expensive
   call to class_getInstanceVariable that is done by
   object_getInstanceVariable.  */
objc_EXPORT id object_getIvar (id object, Ivar variable);

/* Set the value of an instance variable of type 'id' of the object
   'object'.  This is faster than object_setInstanceVariable if you
   already have the instance variable because it avoids the expensive
   call to class_getInstanceVariable that is done by
   object_setInstanceVariable.  */
objc_EXPORT void object_setIvar (id object, Ivar variable, id value);

/* Return the name of the instance variable.  Return NULL if
   'variable' is NULL.  */
objc_EXPORT const char * ivar_getName (Ivar variable);

/* Return the offset of the instance variable from the start of the
   object data.  Return 0 if 'variable' is NULL.  */
objc_EXPORT ptrdiff_t ivar_getOffset (Ivar variable);

/* Return the type encoding of the variable.  Return NULL if
   'variable' is NULL.  */
objc_EXPORT const char * ivar_getTypeEncoding (Ivar variable);

/* Return all the instance variables of the class.  The return value
   of the function is a pointer to an area, allocated with malloc(),
   that contains all the instance variables of the class.  It does not
   include instance variables of superclasses.  The list is terminated
   by NULL.  Optionally, if you pass a non-NULL
   'numberOfReturnedIvars' pointer, the unsigned int that it points to
   will be filled with the number of instance variables returned.  */
objc_EXPORT Ivar * class_copyIvarList (Class class_, unsigned int *numberOfReturnedIvars);


/** Implementation: the following functions are in class.c.  */

/* Compatibility Note: The Apple/NeXT runtime does not have
   objc_get_unknown_class_handler and
   objc_setGetUnknownClassHandler().  They provide functionality that
   the traditional GNU Objective-C Runtime API used to provide via the
   _objc_lookup_class hook.  */

/* An 'objc_get_unknown_class_handler' function is used by
   objc_getClass() to get a class that is currently unknown to the
   compiler.  You could use it for example to have the class loaded by
   dynamically loading a library.  'class_name' is the name of the
   class.  The function should return the Class object if it manages to
   load the class, and Nil if not.  */
typedef Class (*objc_get_unknown_class_handler)(const char *class_name);

/* Sets a new handler function for getting unknown classes (to be used
   by objc_getClass () and related), and returns the previous one.
   This function is not safe to call in a multi-threaded environment
   because other threads may be trying to use the get unknown class
   handler while you change it!  */
objc_get_unknown_class_handler
objc_setGetUnknownClassHandler (objc_get_unknown_class_handler new_handler);


/* Return the class with name 'name', if it is already registered with
   the runtime.  If it is not registered, and
   objc_setGetUnknownClassHandler() has been called to set a handler
   for unknown classes, the handler is called to give it a chance to
   load the class in some other way.  If the class is not known to the
   runtime and the handler is not set or returns Nil, objc_getClass()
   returns Nil.  */
objc_EXPORT Class objc_getClass (const char *name);

/* Return the class with name 'name', if it is already registered with
   the runtime.  Return Nil if not.  This function does not call the
   objc_get_unknown_class_handler function if the class is not
   found.  */
objc_EXPORT Class objc_lookupClass (const char *name);

/* Return the meta class associated to the class with name 'name', if
   it is already registered with the runtime.  First, it finds the
   class using objc_getClass().  Then, it returns the associated meta
   class.  If the class could not be found using objc_getClass(),
   returns Nil.  */
objc_EXPORT Class objc_getMetaClass (const char *name);

/* This is identical to objc_getClass(), but if the class is not found,
   it aborts the process instead of returning Nil.  */
objc_EXPORT Class objc_getRequiredClass (const char *name);

/* If 'returnValue' is NULL, 'objc_getClassList' returns the number of
   classes currently registered with the runtime.  If 'returnValue' is
   not NULL, it should be a (Class *) pointer to an area of memory
   which can contain up to 'maxNumberOfClassesToReturn' Class records.
   'objc_getClassList' will fill the area pointed to by 'returnValue'
   with all the Classes registered with the runtime (or up to
   maxNumberOfClassesToReturn if there are more than
   maxNumberOfClassesToReturn).  The function return value is the
   number of classes actually returned in 'returnValue'.  */
objc_EXPORT int objc_getClassList (Class *returnValue, int maxNumberOfClassesToReturn); 

/* Compatibility Note: The Apple/NeXT runtime also has

    Class objc_getFutureClass (const char *name);
    void objc_setFutureClass (Class class_, const char *name);

   the documentation is unclear on what they are supposed to do, and
   the GNU Objective-C Runtime currently does not provide them.  */

/* Return the name of the class 'class_', or the string "nil" if the
   class_ is Nil.  */
objc_EXPORT const char * class_getName (Class class_);

/* Return YES if 'class_' is a meta class, and NO if not.  If 'class_'
   is Nil, return NO.  */
objc_EXPORT BOOL class_isMetaClass (Class class_);

/* Return the superclass of 'class_'.  If 'class_' is Nil, or it is a root
   class, return Nil.

   TODO: It may be worth to define this inline, since it is usually
   used in loops when traversing the class hierarchy.  */
objc_EXPORT Class class_getSuperclass (Class class_);

/* Return the 'version' number of the class, which is an integer that
   can be used to track changes in the class API, methods and
   variables.  If class_ is Nil, return 0.  If class_ is not Nil, the
   version is 0 unless class_setVersion() has been called to set a
   different one.

   Please note that internally the version is a long, but the API only
   allows you to set and retrieve int values.  */
objc_EXPORT int class_getVersion (Class class_);

/* Set the 'version' number of the class, which is an integer that can
   be used to track changes in the class API, methods and variables.
   If 'class_' is Nil, does nothing.

   This is typically used internally by "Foundation" libraries such as
   GNUstep Base to support serialization / deserialization of objects
   that work across changes in the classes.  If you are using such a
   library, you probably want to use their versioning API, which may
   be based on this one, but is integrated with the rest of the
   library.

   Please note that internally the version is a long, but the API only
   allows you to set and retrieve int values.  */
objc_EXPORT void class_setVersion (Class class_, int version);

/* Return the size in bytes (a byte is the size of a char) of an
   instance of the class.  If class_ is Nil, return 0; else it return
   a non-zero number (since the 'isa' instance variable is required
   for all classes).  */
objc_EXPORT size_t class_getInstanceSize (Class class_);


/** Implementation: the following functions are in sendmsg.c.  */

/* Return the instance method with selector 'selector' of class
   'class_', or NULL if the class (or one of its superclasses) does
   not implement the method.  Return NULL if class_ is Nil or selector
   is NULL.  */
objc_EXPORT Method class_getInstanceMethod (Class class_, SEL selector);

/* Return the class method with selector 'selector' of class 'class_',
   or NULL if the class (or one of its superclasses) does not
   implement the method.  Return NULL if class_ is Nil or selector is
   NULL.  */
objc_EXPORT Method class_getClassMethod (Class class_, SEL selector);

/* Return the IMP (pointer to the function implementing a method) for
   the instance method with selector 'selector' in class 'class_'.
   This is the same routine that is used while messaging, and should
   be very fast.  Note that you most likely would need to cast the
   return function pointer to a function pointer with the appropriate
   arguments and return type before calling it.  To get a class
   method, you can pass the meta-class as the class_ argument (ie, use
   class_getMethodImplementation (object_getClass (class_),
   selector)).  Return NULL if class_ is Nil or selector is NULL.  */
objc_EXPORT IMP class_getMethodImplementation (Class class_, SEL selector);

/* Compatibility Note: the Apple/NeXT runtime has the function
   class_getMethodImplementation_stret () which currently does not
   exist on the GNU runtime because the messaging implementation is
   different.  */

/* Return YES if class 'class_' has an instance method implementing
   selector 'selector', and NO if not.  Return NO if class_ is Nil or
   selector is NULL.  If you need to check a class method, use the
   meta-class as the class_ argument (ie, use class_respondsToSelector
   (object_getClass (class_), selector)).  */
objc_EXPORT BOOL class_respondsToSelector (Class class_, SEL selector);


/** Implementation: the following functions are in methods.c.  */

/* Return the selector for method 'method'.  Return NULL if 'method'
   is NULL.

   This function is misnamed; it should be called
   'method_getSelector'.  To get the actual name, get the selector,
   then the name from the selector (ie, use sel_getName
   (method_getName (method))).  */
objc_EXPORT SEL method_getName (Method method);

/* Return the IMP of the method.  Return NULL if 'method' is NULL.  */
objc_EXPORT IMP method_getImplementation (Method method);

/* Return the type encoding of the method.  Return NULL if 'method' is
   NULL.  */
objc_EXPORT const char * method_getTypeEncoding (Method method);

/* Return a method description for the method.  Return NULL if
   'method' is NULL.  */
objc_EXPORT struct objc_method_description * method_getDescription (Method method);

/* Return all the instance methods of the class.  The return value of
   the function is a pointer to an area, allocated with malloc(), that
   contains all the instance methods of the class.  It does not
   include instance methods of superclasses.  The list is terminated
   by NULL.  Optionally, if you pass a non-NULL
   'numberOfReturnedMethods' pointer, the unsigned int that it points
   to will be filled with the number of instance methods returned.  To
   get the list of class methods, pass the meta-class in the 'class_'
   argument, (ie, use class_copyMethodList (object_getClass (class_),
   &numberOfReturnedMethods)).  */
objc_EXPORT Method * class_copyMethodList (Class class_, unsigned int *numberOfReturnedMethods);


/** Implementation: the following functions are in encoding.c.  */

/* Return the number of arguments that the method 'method' expects.
   Note that all methods need two implicit arguments ('self' for the
   receiver, and '_cmd' for the selector).  Return 0 if 'method' is
   NULL.  */
objc_EXPORT unsigned int method_getNumberOfArguments (Method method);

/* Return the string encoding for the return type of method 'method'.
   The string is a standard NULL-terminated string in an area of
   memory allocated with malloc(); you should free it with free() when
   you finish using it.  Return an empty string if method is NULL.  */
objc_EXPORT char * method_copyReturnType (Method method);

/* Return the string encoding for the argument type of method
   'method', argument number 'argumentNumber' ('argumentNumber' is 0
   for self, 1 for _cmd, and 2 or more for the additional arguments if
   any).  The string is a standard NULL-terminated string in an area
   of memory allocated with malloc(); you should free it with free()
   when you finish using it.  Return an empty string if method is NULL
   or if 'argumentNumber' refers to a non-existing argument.  */
objc_EXPORT char * method_copyArgumentType (Method method, unsigned int argumentNumber);

/* Return the string encoding for the return type of method 'method'.
   The string is returned by copying it into the supplied
   'returnValue' string, which is of size 'returnValueSize'.  No more
   than 'returnValueSize' characters are copied; if the encoding is
   smaller than 'returnValueSize', the rest of 'returnValue' is filled
   with NULLs.  If it is bigger, it is truncated (and would not be
   NULL-terminated).  You should supply a big enough
   'returnValueSize'.  If the method is NULL, returnValue is set to a
   string of NULLs.  */
objc_EXPORT void method_getReturnType (Method method, char *returnValue, 
				       size_t returnValueSize);

/* Return the string encoding for the argument type of method
   'method', argument number 'argumentNumber' ('argumentNumber' is 0
   for self, 1 for _cmd, and 2 or more for the additional arguments if
   any).  The string is returned by copying it into the supplied
   'returnValue' string, which is of size 'returnValueSize'.  No more
   than 'returnValueSize' characters are copied; if the encoding is
   smaller than 'returnValueSize', the rest of 'returnValue' is filled
   with NULLs.  If it is bigger, it is truncated (and would not be
   NULL-terminated).  You should supply a big enough
   'returnValueSize'.  If the method is NULL, returnValue is set to a
   string of NULLs.  */
objc_EXPORT void method_getArgumentType (Method method, unsigned int argumentNumber,
					 char *returnValue, size_t returnValueSize);



/** Implementation: the following functions are in protocols.c.  */

/* Return the protocol with name 'name', or nil if it the protocol is
   not known to the runtime.  */
objc_EXPORT Protocol *objc_getProtocol (const char *name);

/* Return all the protocols known to the runtime.  The return value of
   the function is a pointer to an area, allocated with malloc(), that
   contains all the protocols known to the runtime; the list is
   terminated by NULL.  You should free this area using free() once
   you no longer need it.  Optionally, if you pass a non-NULL
   'numberOfReturnedProtocols' pointer, the unsigned int that it
   points to will be filled with the number of protocols returned.  If
   there are no protocols known to the runtime, NULL is returned.  */
objc_EXPORT Protocol **objc_copyProtocolList (unsigned int *numberOfReturnedProtocols);

/* Add a protocol to a class, and return YES if it was done
   succesfully, and NO if not.  At the moment, NO should only happen
   if class_ or protocol are nil, if the protocol is not a Protocol
   object or if the class already conforms to the protocol.  */
objc_EXPORT BOOL class_addProtocol (Class class_, Protocol *protocol);

/* Return YES if the class 'class_' conforms to Protocol 'protocol',
   and NO if not.  */
objc_EXPORT BOOL class_conformsToProtocol (Class class_, Protocol *protocol);

/* Return all the protocols that the class conforms to.  The return
   value of the function is a pointer to an area, allocated with
   malloc(), that contains all the protocols formally adopted by the
   class.  It does not include protocols adopted by superclasses.  The
   list is terminated by NULL.  Optionally, if you pass a non-NULL
   'numberOfReturnedProtocols' pointer, the unsigned int that it
   points to will be filled with the number of protocols returned.  */
objc_EXPORT Protocol **class_copyProtocolList (Class class_, unsigned int *numberOfReturnedProtocols);

/* Return YES if protocol 'protocol' conforms to protocol
   'anotherProtocol', and NO if not.  Note that if one of the two
   protocols is nil, it returns NO.  */
objc_EXPORT BOOL protocol_conformsToProtocol (Protocol *protocol, Protocol *anotherProtocol);

/* Return YES if protocol 'protocol' is the same as protocol
   'anotherProtocol', and 'NO' if not.  Note that it returns YES if
   the two protocols are both nil.  */
objc_EXPORT BOOL protocol_isEqual (Protocol *protocol, Protocol *anotherProtocol);

/* Return the name of protocol 'protocol'.  If 'protocol' is nil or is
   not a Protocol, return NULL.  */
objc_EXPORT const char *protocol_getName (Protocol *protocol);

/* Return the method description for the method with selector
   'selector' in protocol 'protocol'; if 'requiredMethod' is YES, the
   function searches the list of required methods; if NO, the list of
   optional methods.  If 'instanceMethod' is YES, the function search
   for an instance method; if NO, for a class method.  If there is no
   matching method, an objc_method_description structure with both
   name and types set to NULL is returned.  This function will only
   find methods that are directly declared in the protocol itself, not
   in other protocols that this protocol adopts.

   Note that the traditional ABI does not store the list of optional
   methods of a protocol in a compiled module, so the traditional ABI
   will always return (NULL, NULL) when requiredMethod == NO.  */
objc_EXPORT struct objc_method_description protocol_getMethodDescription (Protocol *protocol, 
									  SEL selector,
									  BOOL requiredMethod,
									  BOOL instanceMethod);

/* Return the method descriptions of all the methods of the protocol.
   The return value of the function is a pointer to an area, allocated
   with malloc(), that contains all the method descriptions of the
   methods of the protocol.  It does not recursively include methods
   of the protocols adopted by this protocol.  The list is terminated
   by a NULL objc_method_description (one with both fields set to
   NULL).  Optionally, if you pass a non-NULL
   'numberOfReturnedMethods' pointer, the unsigned int that it points
   to will be filled with the number of properties returned.

   Note that the traditional ABI does not store the list of optional
   methods of a protocol in a compiled module, so the traditional ABI
   will always return an empty list if requiredMethod is set to
   NO.  */
objc_EXPORT struct objc_method_description *protocol_copyMethodDescriptionList (Protocol *protocol,
										BOOL requiredMethod,
										BOOL instanceMethod,
										unsigned int *numberOfReturnedMethods);

/* Return the property with name 'propertyName' of the protocol
   'protocol'.  If 'requiredProperty' is YES, the function searches
   the list of required properties; if NO, the list of optional
   properties.  If 'instanceProperty' is YES, the function searches
   the list of instance properties; if NO, the list of class
   properties.  At the moment, optional properties and class
   properties are not part of the Objective-C language, so both
   'requiredProperty' and 'instanceProperty' should be set to YES.
   This function returns NULL if the required property can not be
   found.

   Note that the traditional ABI does not store the list of properties
   of a protocol in a compiled module, so the traditional ABI will
   always return NULL.  */
objc_EXPORT Property protocol_getProperty (Protocol *protocol, const char *propertyName, 
					   BOOL requiredProperty, BOOL instanceProperty);

/* Return all the properties of the protocol.  The return value of the
   function is a pointer to an area, allocated with malloc(), that
   contains all the properties of the protocol.  It does not
   recursively include properties of the protocols adopted by this
   protocol.  The list is terminated by NULL.  Optionally, if you pass
   a non-NULL 'numberOfReturnedProperties' pointer, the unsigned int
   that it points to will be filled with the number of properties
   returned.

   Note that the traditional ABI does not store the list of properties
   of a protocol in a compiled module, so the traditional ABI will
   always return NULL and store 0 in numberOfReturnedProperties.  */
objc_EXPORT Property *protocol_copyPropertyList (Protocol *protocol, unsigned int *numberOfReturnedProperties);

/* Return all the protocols that the protocol conforms to.  The return
   value of the function is a pointer to an area, allocated with
   malloc(), that contains all the protocols formally adopted by the
   protocol.  It does not recursively include protocols adopted by the
   protocols adopted by this protocol.  The list is terminated by
   NULL.  Optionally, if you pass a non-NULL
   'numberOfReturnedProtocols' pointer, the unsigned int that it
   points to will be filled with the number of protocols returned.  */
objc_EXPORT Protocol **protocol_copyProtocolList (Protocol *protocol, unsigned int *numberOfReturnedProtocols);


/* TODO: Add all the other functions in the API.  */


/** Implementation: the following functions are in objc-foreach.c.  */

/* 'objc_enumerationMutation()' is called when a collection is
   mutated while being "fast enumerated".  That is a hard error, and
   objc_enumerationMutation is called to deal with it.  'collection'
   is the collection object that was mutated during an enumeration.

   objc_enumerationMutation() will invoke the mutation handler if any
   is set.  Then, it will abort the program.

   Compatibility note: the Apple runtime will not abort the program
   after calling the mutation handler.  */
objc_EXPORT void objc_enumerationMutation (id collection);

/* 'objc_set_enumeration_mutation_handler' can be used to set a
   function that will be called (instead of aborting) when a fast
   enumeration is mutated during enumeration.  The handler will be
   called with the 'collection' being mutated as the only argument and
   it should not return; it should either exit the program, or could
   throw an exception.  The recommended implementation is to throw an
   exception - the user can then use exception handlers to deal with
   it.

   This function is not thread safe (other threads may be trying to
   invoke the enumeration mutation handler while you are changing it!)
   and should be called during during the program initialization
   before threads are started.  It is mostly reserved for "Foundation"
   libraries; in the case of GNUstep, GNUstep Base may be using this
   function to improve the standard enumeration mutation handling.
   You probably shouldn't use this function unless you are writing
   your own Foundation library.  */
objc_EXPORT void objc_setEnumerationMutationHandler (void (*handler)(id));

/* This structure (used during fast enumeration) is automatically
   defined by the compiler (it is as if this definition was always
   included in all Objective-C files).  Note that it is usually
   defined again with the name of NSFastEnumeration by "Foundation"
   libraries such as GNUstep Base.  And if NSFastEnumeration is
   defined, the compiler will use it instead of
   __objcFastEnumerationState when doing fast enumeration.  */
/*
struct __objcFastEnumerationState
{
  unsigned long state;
  id            *itemsPtr;
  unsigned long *mutationsPtr;
  unsigned long extra[5];
};
*/


/** Implementation: the following functions are in memory.c.  */

/* Traditional GNU Objective-C Runtime functions that are used for
   memory allocation and disposal.  These functions are used in the
   same way as you use malloc, realloc, calloc and free and make sure
   that memory allocation works properly with the garbage
   collector.

   Compatibility Note: these functions are not available with the
   Apple/NeXT runtime.  */

objc_EXPORT void *objc_malloc(size_t size);

/* FIXME: Shouldn't the following be called objc_malloc_atomic ?  The
   GC function is GC_malloc_atomic() which makes sense.
 */
objc_EXPORT void *objc_atomic_malloc(size_t size);

objc_EXPORT void *objc_realloc(void *mem, size_t size);

objc_EXPORT void *objc_calloc(size_t nelem, size_t size);

objc_EXPORT void objc_free(void *mem);


/** Implementation: the following functions are in encoding.c.  */

/* Traditional GNU Objective-C Runtime functions that are currently
   used to implement method forwarding.

   Compatibility Note: these functions are not available with the
   Apple/NeXT runtime.  */

/* Return the size of a variable which has the specified 'type'
   encoding.  */
int objc_sizeof_type (const char *type);

/* Return the align of a variable which has the specified 'type'
   encoding.  */
int objc_alignof_type (const char *type);

/* Return the aligned size of a variable which has the specified
   'type' encoding.  The aligned size is the size rounded up to the
   nearest alignment.  */
int objc_aligned_size (const char *type);

/* Return the promoted size of a variable which has the specified
   'type' encoding.  This is the size rounded up to the nearest
   integral of the wordsize, taken to be the size of a void *.  */
int objc_promoted_size (const char *type);


/* The following functions are used when parsing the type encoding of
   methods, to skip over parts that are ignored.  They take as
   argument a pointer to a location inside the type encoding of a
   method (which is a string) and return a new pointer, pointing to a
   new location inside the string after having skipped the unwanted
   information.  */

/* Skip some type qualifiers (_C_CONST, _C_IN, etc).  These may
  eventually precede typespecs occurring in method prototype
  encodings.  */
const char *objc_skip_type_qualifiers (const char *type);

/* Skip one typespec element (_C_CLASS, _C_SEL, etc).  If the typespec
  is prepended by type qualifiers, these are skipped as well.  */
const char *objc_skip_typespec (const char *type);

/* Skip an offset.  */
const char *objc_skip_offset (const char *type);

/* Skip an argument specification (ie, skipping a typespec, which may
   include qualifiers, and an offset too).  */
const char *objc_skip_argspec (const char *type);

/* Read type qualifiers (_C_CONST, _C_IN, etc) from string 'type'
   (stopping at the first non-type qualifier found) and return an
   unsigned int which is the logical OR of all the corresponding flags
   (_F_CONST, _F_IN etc).  */
unsigned objc_get_type_qualifiers (const char *type);


/* Note that the following functions work for very simple structures,
   but get easily confused by more complicated ones (for example,
   containing vectors).  A better solution is required.
*/

/* The following three functions can be used to determine how a
   structure is laid out by the compiler. For example:

  struct objc_struct_layout layout;
  int i;

  objc_layout_structure (type, &layout);
  while (objc_layout_structure_next_member (&layout))
    {
      int position, align;
      const char *type;

      objc_layout_structure_get_info (&layout, &position, &align, &type);
      printf ("element %d has offset %d, alignment %d\n",
              i++, position, align);
    }

  These functions are used by objc_sizeof_type and objc_alignof_type
  functions to compute the size and alignment of structures. The
  previous method of computing the size and alignment of a structure
  was not working on some architectures, particulary on AIX, and in
  the presence of bitfields inside the structure.  */
struct objc_struct_layout
{
  const char *original_type;
  const char *type;
  const char *prev_type;
  unsigned int record_size;
  unsigned int record_align;
};

void objc_layout_structure (const char *type,
                            struct objc_struct_layout *layout);
BOOL  objc_layout_structure_next_member (struct objc_struct_layout *layout);
void objc_layout_finish_structure (struct objc_struct_layout *layout,
                                   unsigned int *size,
                                   unsigned int *align);
void objc_layout_structure_get_info (struct objc_struct_layout *layout,
                                     unsigned int *offset,
                                     unsigned int *align,
                                     const char **type);

#endif
