// Filename: pStatCollectorDef.cxx
// Created by:  drose (09Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "pStatCollectorDef.h"

#include <datagram.h>
#include <datagramIterator.h>


////////////////////////////////////////////////////////////////////
//     Function: PStatCollectorDef::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PStatCollectorDef::
PStatCollectorDef() {
  _index = 0;
  _parent_index = 0;
  _suggested_color.set(0.0, 0.0, 0.0);
  _sort = -1;
  _suggested_scale = 0.0;
  _factor = 1.0;
  _is_active = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollectorDef::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PStatCollectorDef::
PStatCollectorDef(int index, const string &name) : 
  _index(index),
  _name(name)
{
  _parent_index = 0;
  _suggested_color.set(0.0, 0.0, 0.0);
  _sort = -1;
  _suggested_scale = 0.0;
  _factor = 1.0;
  _is_active = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollectorDef::set_parent
//       Access: Public
//  Description: This is normally called only by the PStatClient when
//               the new PStatCollectorDef is created; it sets the
//               parent of the CollectorDef and inherits whatever
//               properties are appropriate.
////////////////////////////////////////////////////////////////////
void PStatCollectorDef::
set_parent(const PStatCollectorDef &parent) {
  _parent_index = parent._index;
  _level_units = parent._level_units;
  _suggested_scale = parent._suggested_scale;
  _factor = parent._factor;
  _is_active = parent._is_active;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollectorDef::write_datagram
//       Access: Public
//  Description: Writes the definition of the collectorDef to the
//               datagram.
////////////////////////////////////////////////////////////////////
void PStatCollectorDef::
write_datagram(Datagram &destination) const {
  destination.add_int16(_index);
  destination.add_string(_name);
  destination.add_int16(_parent_index);
  _suggested_color.write_datagram(destination);
  destination.add_int16(_sort);
  destination.add_string(_level_units);
  destination.add_float32(_suggested_scale);
  destination.add_float32(_factor);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollectorDef::read_datagram
//       Access: Public
//  Description: Extracts the collectorDef definition from the datagram.
////////////////////////////////////////////////////////////////////
void PStatCollectorDef::
read_datagram(DatagramIterator &source, PStatClientVersion *) {
  _index = source.get_int16();
  _name = source.get_string();
  _parent_index = source.get_int16();
  _suggested_color.read_datagram(source);
  _sort = source.get_int16();
  _level_units = source.get_string();
  _suggested_scale = source.get_float32();
  _factor = source.get_float32();
}
