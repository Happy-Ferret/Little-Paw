// Filename: typedObject.h
// Created by:  drose (11May01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef TYPEDOBJECT_H
#define TYPEDOBJECT_H

#include "dtoolbase.h"

#include "typeHandle.h"
#include "register_type.h"
#include "memoryBase.h"

#include <set>

////////////////////////////////////////////////////////////////////
//       Class : TypedObject
// Description : This is an abstract class that all classes which
//               use TypeHandle, and also provide virtual functions to
//               support polymorphism, should inherit from.  Each
//               derived class should define get_type(), which should
//               return the specific type of the derived class.
//               Inheriting from this automatically provides support
//               for is_of_type() and is_exact_type().
//               
//               All classes that inherit directly or indirectly from
//               TypedObject should redefine get_type() and
//               force_init_type(), as shown below.  Some classes that
//               do not inherit from TypedObject may still declare
//               TypeHandles for themselves by defining methods called
//               get_class_type() and init_type().  Classes such as
//               these may serve as base classes, but the dynamic type
//               identification system will be limited.  Classes that
//               do not inherit from TypedObject need not define the
//               virtual functions get_type() and force_init_type()
//               (or any other virtual functions).
//               
//               There is a specific layout for defining the
//               overrides from this class.  Keeping the definitions
//               formatted just like these examples will allow
//               someone in the future to use a sed (or similar)
//               script to make global changes, if necessary.  Avoid
//               rearranging the braces or the order of the functions
//               unless you're ready to change them in every file all
//               at once.
//               
//               What follows are some examples that can be used in
//               new classes that you create.
//               
//               ---------------------------------------------------
//               In the class definition (.h file)
//               ---------------------------------------------------
//               
//               public:
//                 static TypeHandle get_class_type() {
//                   return _type_handle;
//                 }
//                 static void init_type() {
//                   <<<BaseClassOne>>>::init_type();
//                   <<<BaseClassTwo>>>::init_type();
//                   <<<BaseClassN>>>::init_type();
//                   register_type(_type_handle, "<<<ThisClassStringName>>>",
//                                 <<<BaseClassOne>>>::get_class_type(),
//                                 <<<BaseClassTwo>>>::get_class_type(),
//                                 <<<BaseClassN>>>::get_class_type());
//                 }
//                 virtual TypeHandle get_type() const {
//                   return get_class_type();
//                 }
//                 virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
//               
//               private:
//                 static TypeHandle _type_handle;
//               
//               
//               ----------------------
//               In the class .cxx file
//               ----------------------
//               
//               TypeHandle <<<ThisClassStringName>>>::_type_handle;
//               
//               
//               ----------------------------------------------
//               In the class config_<<<PackageName>>>.cxx file
//               ----------------------------------------------
//               
//               ConfigureFn(config_<<<PackageName>>>) {
//                 <<<ClassOne>>>::init_type();
//                 <<<ClassTwo>>>::init_type();
//                 <<<ClassN>>>::init_type();
//               }
//               
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG TypedObject : public MemoryBase {
public:
  INLINE TypedObject();
  INLINE TypedObject(const TypedObject &copy);
  INLINE void operator = (const TypedObject &copy);

PUBLISHED:
  // A virtual destructor is just a good idea.
  virtual ~TypedObject();

  // Derived classes should override this function to return
  // get_class_type().
  virtual TypeHandle get_type() const=0;

  INLINE int get_type_index() const;
  INLINE bool is_of_type(TypeHandle handle) const;
  INLINE bool is_exact_type(TypeHandle handle) const;

public:
  INLINE int get_best_parent_from_Set(const std::set<int> &) const;

  // Derived classes should override this function to call
  // init_type().  It will only be called in error situations when the
  // type was for some reason not properly initialized.
  virtual TypeHandle force_init_type()=0;

  // This pair of methods exists mainly for the convenience of
  // unambiguous upcasting.  Interrogate generates code to call this
  // method instead of making an explicit cast to (TypedObject *);
  // this allows classes who multiply inherit from TypedObejct to
  // override these methods and disambiguate the cast.  It doesn't
  // have to be a virtual method, since this is just a static upcast.
  INLINE TypedObject *as_typed_object();
  INLINE const TypedObject *as_typed_object() const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "typedObject.I"

#endif
