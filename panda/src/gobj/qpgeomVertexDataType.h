// Filename: qpgeomVertexDataType.h
// Created by:  drose (06Mar05)
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

#ifndef qpGEOMVERTEXDATATYPE_H
#define qpGEOMVERTEXDATATYPE_H

#include "pandabase.h"
#include "internalName.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : qpGeomVertexDataType
// Description : This defines how a single data type is interleaved
//               within a vertex array stored within a Geom.  The
//               GeomVertexArrayFormat class maintains a list of these
//               to completely define a particular array structure.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomVertexDataType {
PUBLISHED:
  enum NumericType {
    NT_uint8,
    NT_packed_argb,  // DirectX style
    NT_float,
  };

  INLINE qpGeomVertexDataType(const InternalName *name, int num_components,
                              NumericType numeric_type, int start);
  INLINE qpGeomVertexDataType(const qpGeomVertexDataType &copy);
  INLINE void operator = (const qpGeomVertexDataType &copy);

  static const qpGeomVertexDataType &error();

  INLINE const InternalName *get_name() const;
  INLINE int get_num_components() const;
  INLINE int get_num_values() const;
  INLINE NumericType get_numeric_type() const;
  INLINE int get_start() const;
  INLINE int get_component_bytes() const;
  INLINE int get_total_bytes() const;

  INLINE bool overlaps_with(int start_byte, int num_bytes) const;

  void output(ostream &out) const;

public:
  INLINE int compare_to(const qpGeomVertexDataType &other) const;
  INLINE bool operator == (const qpGeomVertexDataType &other) const;
  INLINE bool operator != (const qpGeomVertexDataType &other) const;
  INLINE bool operator < (const qpGeomVertexDataType &other) const;

  void copy_records(unsigned char *to, int to_stride,
                    const unsigned char *from, int from_stride,
                    const qpGeomVertexDataType *from_type,
                    int num_records) const;

private:
  void copy_no_convert(unsigned char *to, int to_stride,
                       const unsigned char *from, int from_stride,
                       const qpGeomVertexDataType *from_type,
                       int num_records) const;
  void copy_argb_to_uint8(unsigned char *to, int to_stride,
                          const unsigned char *from, int from_stride,
                          const qpGeomVertexDataType *from_type,
                          int num_records) const;
  void copy_uint8_to_argb(unsigned char *to, int to_stride,
                          const unsigned char *from, int from_stride,
                          const qpGeomVertexDataType *from_type,
                          int num_records) const;

  void copy_generic(unsigned char *to, int to_stride,
                    const unsigned char *from, int from_stride,
                    const qpGeomVertexDataType *from_type,
                    int num_records) const;

  float get_value(const unsigned char *data, int n) const;
  void set_value(unsigned char *data, int n, float value) const;

private:
  CPT(InternalName) _name;
  int _num_components;
  int _num_values;
  NumericType _numeric_type;
  int _start;
  int _component_bytes;
  int _total_bytes;
};

INLINE ostream &operator << (ostream &out, const qpGeomVertexDataType &obj);

#include "qpgeomVertexDataType.I"

#endif
