// Filename: qpqpFindApproxPath.h
// Created by:  drose (13Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef qpFINDAPPROXPATH_H
#define qpFINDAPPROXPATH_H

#include "pandabase.h"

#include "pvector.h"

class PandaNode;

////////////////////////////////////////////////////////////////////
//       Class : qpFindApproxPath
// Description : This class is local to this package only; it doesn't
//               get exported.  It chops a string path, as supplied to
//               find_up() or find_down(), and breaks it up into its
//               component pieces.
////////////////////////////////////////////////////////////////////
class qpFindApproxPath {
public:
  INLINE qpFindApproxPath();

  bool add_string(const string &str_path);
  bool add_flags(const string &str_flags);
  bool add_component(string str_component);

  INLINE void add_match_name(const string &name, int flags);
  INLINE void add_match_name_glob(const string &glob, int flags);
  INLINE void add_match_exact_type(TypeHandle type, int flags);
  INLINE void add_match_inexact_type(TypeHandle type, int flags);
  INLINE void add_match_one(int flags);
  INLINE void add_match_many(int flags);
  INLINE void add_match_pointer(PandaNode *pointer, int flags);

  INLINE int get_num_components() const;
  INLINE bool is_component_match_many(int index) const;
  INLINE bool matches_component(int index, PandaNode *node) const;
  INLINE bool matches_stashed(int index) const;

  INLINE bool return_hidden() const;
  INLINE bool return_stashed() const;

  void output(ostream &out) const;
  INLINE void output_component(ostream &out, int index) const;

#ifndef WIN32_VC
// Visual C++ won't let us define the ostream operator functions for
// these guys if they're private--even though we declare them friends.
private:
#endif
  enum ComponentType {
    CT_match_name,
    CT_match_name_glob,
    CT_match_exact_type,
    CT_match_inexact_type,
    CT_match_one,
    CT_match_many,
    CT_match_pointer
  };
  enum ComponentFlags {
    CF_stashed        = 0x001,
  };

  class Component {
  public:
    bool matches(PandaNode *node) const;
    void output(ostream &out) const;

    ComponentType _type;
    string _name;
    TypeHandle _type_handle;
    PandaNode *_pointer;
    int _flags;
  };

  typedef pvector<Component> Path;
  Path _path;

  bool _return_hidden;
  bool _return_stashed;

friend ostream &operator << (ostream &, qpFindApproxPath::ComponentType);
friend INLINE ostream &operator << (ostream &, const qpFindApproxPath::Component &);
};

ostream &
operator << (ostream &out, qpFindApproxPath::ComponentType type);

INLINE ostream &
operator << (ostream &out, const qpFindApproxPath::Component &component) {
  component.output(out);
  return out;
}

INLINE ostream &
operator << (ostream &out, const qpFindApproxPath &path) {
  path.output(out);
  return out;
}

#include "qpfindApproxPath.I"

#endif
