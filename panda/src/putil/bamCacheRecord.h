// Filename: bamCacheRecord.h
// Created by:  drose (08Jun06)
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

#ifndef BAMCACHERECORD_H
#define BAMCACHERECORD_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"

class BamWriter;
class BamReader;
class Datagram;
class DatagramIterator;
class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : BamCacheRecord
// Description : An instance of this class is written to the front of
//               a Bam or Txo file to make the file a cached instance
//               of some other loadable resource.  This record
//               contains information needed to test the validity of
//               the cache.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BamCacheRecord : public TypedWritableReferenceCount {
private:
  BamCacheRecord();
  BamCacheRecord(const Filename &source_pathname, 
                 const Filename &cache_filename);

PUBLISHED:
  virtual ~BamCacheRecord();

  INLINE const Filename &get_source_pathname() const;
  INLINE const Filename &get_cache_filename() const;
  INLINE time_t get_recorded_time() const;

  INLINE int get_num_dependent_files() const;
  INLINE const Filename &get_dependent_pathname(int n) const;

  bool dependents_unchanged() const;
  void clear_dependent_files();
  void add_dependent_file(const Filename &pathname);

  INLINE bool has_data() const;
  INLINE void clear_data();
  INLINE const TypedWritable *get_data() const;
  INLINE TypedWritable *extract_data();
  INLINE void set_data(TypedWritable *data, bool owns_pointer);

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  static string format_timestamp(time_t timestamp);

  Filename _source_pathname;
  Filename _cache_filename;
  time_t _recorded_time;

  class DependentFile {
  public:
    Filename _pathname;
    time_t _timestamp;
    off_t _size;
  };

  typedef pvector<DependentFile> DependentFiles;
  DependentFiles _files;
  
  // The following are not recorded to disk; they are preserved
  // in-memory only for the current session.
  Filename _cache_pathname;
  TypedWritable *_data;
  bool _owns_pointer;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "BamCacheRecord",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class BamCache;
};

INLINE ostream &operator << (ostream &out, const BamCacheRecord &record) {
  record.output(out);
  return out;
}

#include "bamCacheRecord.I"

#endif
