// Filename: loaderFileTypeEgg.h
// Created by:  drose (20Jun00)
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

#ifndef LOADERFILETYPEEGG_H
#define LOADERFILETYPEEGG_H

#include "pandabase.h"

#include "loaderFileType.h"

////////////////////////////////////////////////////////////////////
//       Class : LoaderFileTypeEgg
// Description : This defines the Loader interface to read Egg files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG LoaderFileTypeEgg : public LoaderFileType {
public:
  LoaderFileTypeEgg();

  virtual string get_name() const;
  virtual string get_extension() const;

  virtual void resolve_filename(Filename &path) const;
  virtual PT(PandaNode) load_file(const Filename &path, bool report_errors) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LoaderFileType::init_type();
    register_type(_type_handle, "LoaderFileTypeEgg",
                  LoaderFileType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif

