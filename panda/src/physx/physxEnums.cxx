// Filename: physxEnums.cxx
// Created by:  enn0x (23Sep09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "physxEnums.h"

#include "string_utils.h"
#include "config_util.h"

ostream &
operator << (ostream &out, PhysxEnums::PhysxUpAxis axis) {

  switch (axis) {
  case PhysxEnums::X_up:
    return out << "x";

  case PhysxEnums::Y_up:
    return out << "y";

  case PhysxEnums::Z_up:
    return out << "z";
  }

  return out << "**invalid PhysxEnums::PhysxUpAxis value: (" << (int)axis << ")**";
}

istream &
operator >> (istream &in, PhysxEnums::PhysxUpAxis &axis) {

  string word;
  in >> word;

  if (cmp_nocase(word, "x") == 0) {
    axis = PhysxEnums::X_up;
  } 
  else if (cmp_nocase(word, "y") == 0) {
    axis = PhysxEnums::Y_up;
  }
  else if (cmp_nocase(word, "z") == 0) {
    axis = PhysxEnums::Z_up;
  }
  else {
    physx_cat->error()
      << "Invalid up-axis string: " << word << "\n";
    axis = PhysxEnums::Z_up;
  }

  return in;
}

