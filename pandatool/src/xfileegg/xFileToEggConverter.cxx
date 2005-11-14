// Filename: xFileToEggConverter.cxx
// Created by:  drose (21Jun01)
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

#include "xFileToEggConverter.h"
#include "xFileMesh.h"
#include "xFileMaterial.h"
#include "xFileAnimationSet.h"
#include "config_xfile.h"

#include "eggData.h"
#include "eggGroup.h"
#include "eggXfmSAnim.h"
#include "eggGroupUniquifier.h"
#include "datagram.h"
#include "eggMaterialCollection.h"
#include "eggTextureCollection.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileToEggConverter::
XFileToEggConverter() {
  _make_char = false;
  _frame_rate = 30.0;
  _x_file = new XFile(true);
  _dart_node = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileToEggConverter::
XFileToEggConverter(const XFileToEggConverter &copy) :
  SomethingToEggConverter(copy),
  _make_char(copy._make_char)
{
  _x_file = new XFile(true);
  _dart_node = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileToEggConverter::
~XFileToEggConverter() {
  close();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::make_copy
//       Access: Public, Virtual
//  Description: Allocates and returns a new copy of the converter.
////////////////////////////////////////////////////////////////////
SomethingToEggConverter *XFileToEggConverter::
make_copy() {
  return new XFileToEggConverter(*this);
}


////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::get_name
//       Access: Public, Virtual
//  Description: Returns the English name of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string XFileToEggConverter::
get_name() const {
  return "DirectX";
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::get_extension
//       Access: Public, Virtual
//  Description: Returns the common extension of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string XFileToEggConverter::
get_extension() const {
  return "x";
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_file
//       Access: Public, Virtual
//  Description: Handles the reading of the input file and converting
//               it to egg.  Returns true if successful, false
//               otherwise.
//
//               This is designed to be as generic as possible,
//               generally in support of run-time loading.
//               Command-line converters may choose to use
//               convert_flt() instead, as it provides more control.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_file(const Filename &filename) {
  close();
  clear_error();

  if (!_x_file->read(filename)) {
    nout << "Unable to open X file: " << filename << "\n";
    return false;
  }

  if (_char_name.empty()) {
    _char_name = filename.get_basename_wo_extension();
  }

  if (_egg_data->get_coordinate_system() == CS_default) {
    _egg_data->set_coordinate_system(CS_yup_left);
  }

  if (!get_toplevel()) {
    return false;
  }

  if (!create_polygons()) {
    return false;
  }

  if (_make_char) {
    // Now make sure that each joint has a unique name.
    EggGroupUniquifier uniquifier(false);
    uniquifier.uniquify(_dart_node);
  }

  if (!create_hierarchy()) {
    return false;
  }

  if (_keep_model && !_keep_animation) {
    strip_nodes(EggTable::get_class_type());
  }
  
  if (_keep_animation && !_keep_model) {
    strip_nodes(EggGroup::get_class_type());
  }
  
  return !had_error();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::close
//       Access: Public
//  Description: Finalizes and closes the file previously opened via
//               convert_file().
////////////////////////////////////////////////////////////////////
void XFileToEggConverter::
close() {
  _x_file->clear();

  // Clean up all the other stuff.
  Meshes::const_iterator mi;
  for (mi = _meshes.begin(); mi != _meshes.end(); ++mi) {
    delete (*mi);
  }
  _meshes.clear();
  
  AnimationSets::const_iterator asi;
  for (asi = _animation_sets.begin(); asi != _animation_sets.end(); ++asi) {
    delete (*asi);
  }
  _animation_sets.clear();

  _joints.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::strip_nodes
//       Access: Public
//  Description: Removes all groups of the given type.  This is used
//               to implement the -anim and -model options.
////////////////////////////////////////////////////////////////////
void XFileToEggConverter::
strip_nodes(TypeHandle t) {
  pvector <EggNode *> garbage;
  EggGroupNode::iterator i;
  for (i=_egg_data->begin(); i!=_egg_data->end(); ++i) {
    EggNode *node = (*i);
    if (node->is_of_type(t)) {
      garbage.push_back(node);
    }
  }
  for (int n=0; n<(int)garbage.size(); n++) {
    _egg_data->remove_child(garbage[n]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::get_dart_node
//       Access: Public
//  Description: Returns the root of the joint hierarchy, if
//               _make_char is true, or NULL otherwise.
////////////////////////////////////////////////////////////////////
EggGroup *XFileToEggConverter::
get_dart_node() const {
  return _dart_node;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::create_unique_texture
//       Access: Public
//  Description: Returns an EggTexture pointer whose properties match
//               that of the the given EggTexture, except for the tref
//               name.
////////////////////////////////////////////////////////////////////
EggTexture *XFileToEggConverter::
create_unique_texture(const EggTexture &copy) {
  return _textures.create_unique_texture(copy, ~EggTexture::E_tref_name);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::create_unique_material
//       Access: Public
//  Description: Returns an EggMaterial pointer whose properties match
//               that of the the given EggMaterial, except for the mref
//               name.
////////////////////////////////////////////////////////////////////
EggMaterial *XFileToEggConverter::
create_unique_material(const EggMaterial &copy) {
  return _materials.create_unique_material(copy, ~EggMaterial::E_mref_name);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::find_joint
//       Access: Public
//  Description: This is called by set_animation_frame, for
//               the purposes of building the frame data for the
//               animation--it needs to know the original rest frame
//               transform.
////////////////////////////////////////////////////////////////////
EggGroup *XFileToEggConverter::
find_joint(const string &joint_name) {
  Joints::iterator ji;
  ji = _joints.find(joint_name);
  if (ji != _joints.end()) {
    EggGroup *joint = (*ji).second;
    if (joint == (EggGroup *)NULL) {
      // An invalid joint detected earlier.
      return NULL;
    }

    return joint;
  }

  // Joint name is unknown.  Issue a warning, then insert NULL into
  // the table so we don't get the same warning again with the next
  // polygon.
  if (_make_char) {
    xfile_cat.warning()
      << "Joint name " << joint_name << " in animation data is undefined.\n";
  }
  _joints[joint_name] = NULL;

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::get_toplevel
//       Access: Private
//  Description: Pulls off all of the top-level objects in the .x file
//               and converts them, and their descendents, to the
//               appropriate egg structures.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
get_toplevel() {
  int num_objects = _x_file->get_num_objects();
  int i;

  // First, make a pass through the toplevel objects and see if we
  // have frames and/or animation.
  _any_frames = false;
  _any_animation = false;
  for (i = 0; i < num_objects; i++) {
    XFileDataNode *child = _x_file->get_object(i);
    if (child->is_standard_object("Frame")) {
      _any_frames = true;
    } else if (child->is_standard_object("AnimationSet")) {
      _any_animation = true;
    }
  }

  // If we have animation, assume we want to convert it as a character.
  if (_any_animation) {
    _make_char = true;
  }

  EggGroupNode *egg_parent = _egg_data;
  
  // If we are converting an animatable model, make an extra node to
  // represent the root of the hierarchy.
  if (_make_char) {
    _dart_node = new EggGroup(_char_name);
    egg_parent->add_child(_dart_node);
    _dart_node->set_dart_type(EggGroup::DT_default);
    egg_parent = _dart_node;
  }

  // Now go back through and convert the objects.
  for (i = 0; i < num_objects; i++) {
    if (!convert_toplevel_object(_x_file->get_object(i), egg_parent)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_toplevel_object
//       Access: Private
//  Description: Converts the indicated object, encountered outside of
//               any Frames, to the appropriate egg structures.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_toplevel_object(XFileDataNode *obj, EggGroupNode *egg_parent) {
  if (obj->is_standard_object("Header")) {
    // Quietly ignore headers.

  } else if (obj->is_standard_object("Material")) {
    // Quietly ignore toplevel materials.  These will presumably be
    // referenced below.

  } else if (obj->is_standard_object("Frame")) {
    if (!convert_frame(obj, egg_parent)) {
      return false;
    }

  } else if (obj->is_standard_object("AnimationSet")) {
    if (!convert_animation_set(obj)) {
      return false;
    }

  } else if (obj->is_standard_object("Mesh")) {
    // If there are any Frames at all in the file, then assume a Mesh
    // at the toplevel is just present to define a reference that will
    // be included below--so we ignore it here.  On the other hand, if
    // the file has no Frames, then a Mesh at the toplevel must be
    // actual geometry, so convert it now.
    if (!_any_frames) {
      if (!convert_mesh(obj, egg_parent)) {
        return false;
      }
    }

  } else {
    if (xfile_cat.is_debug()) {
      xfile_cat.debug()
        << "Ignoring toplevel object of unknown type: "
        << obj->get_template_name() << "\n";
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_object
//       Access: Private
//  Description: Converts the indicated object to the appropriate egg
//               structures.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_object(XFileDataNode *obj, EggGroupNode *egg_parent) {
  if (obj->is_standard_object("Header")) {
    // Quietly ignore headers.

  } else if (obj->is_standard_object("Frame")) {
    if (!convert_frame(obj, egg_parent)) {
      return false;
    }

  } else if (obj->is_standard_object("FrameTransformMatrix")) {
    if (!convert_transform(obj, egg_parent)) {
      return false;
    }

  } else if (obj->is_standard_object("Mesh")) {
    if (!convert_mesh(obj, egg_parent)) {
      return false;
    }

  } else {
    if (xfile_cat.is_debug()) {
      xfile_cat.debug()
        << "Ignoring object of unknown type: "
        << obj->get_template_name() << "\n";
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_frame
//       Access: Private
//  Description: Converts the indicated frame to the appropriate egg
//               structures.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_frame(XFileDataNode *obj, EggGroupNode *egg_parent) {

  string name = obj->get_name();
  EggGroup *group = new EggGroup(name);
  egg_parent->add_child(group);
  
  if (_make_char) {
    group->set_group_type(EggGroup::GT_joint);
    if (name.empty()) {
      // Make up a name for this unnamed joint.
      group->set_name("unnamed");

    } else {
      bool inserted = _joints.insert(Joints::value_type(name, group)).second;
      if (!inserted) {
        xfile_cat.warning()
          << "Nonunique Frame name " << name
          << " encountered; animation will be ambiguous.\n";
      }
    }
  }

  // Now walk through the children of the frame.
  int num_objects = obj->get_num_objects();
  for (int i = 0; i < num_objects; i++) {
    if (!convert_object(obj->get_object(i), group)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_transform
//       Access: Private
//  Description: Reads a transform matrix, a child of a given frame,
//               and applies it to the node.  Normally this can only
//               be done if the node in question is an EggGroup, which
//               should be the case if the transform was a child of a
//               frame.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_transform(XFileDataNode *obj, EggGroupNode *egg_parent) {
  LMatrix4d mat = (*obj)["frameMatrix"]["matrix"].mat4();

  if (egg_parent->is_of_type(EggGroup::get_class_type())) {
    EggGroup *egg_group = DCAST(EggGroup, egg_parent);
    egg_group->set_transform3d(mat);

  } else {
    xfile_cat.error()
      << "Transform " << obj->get_name()
      << " encountered without frame!\n";
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_animation_set
//       Access: Private
//  Description: Begins an AnimationSet.  This is the root of one
//               particular animation (table of frames per joint) to
//               be applied to the model within this file.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_animation_set(XFileDataNode *obj) {
  XFileAnimationSet *animation_set = new XFileAnimationSet();
  animation_set->set_name(obj->get_name());

  // Now walk through the children of the set; each one animates a
  // different joint.
  int num_objects = obj->get_num_objects();
  for (int i = 0; i < num_objects; i++) {
    if (!convert_animation_set_object(obj->get_object(i), *animation_set)) {
      return false;
    }
  }

  _animation_sets.push_back(animation_set);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_animation_set_object
//       Access: Private
//  Description: Converts the indicated object, a child of a
//               AnimationSet.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_animation_set_object(XFileDataNode *obj, 
                             XFileAnimationSet &animation_set) {
  if (obj->is_standard_object("Animation")) {
    if (!convert_animation(obj, animation_set)) {
      return false;
    }

  } else {
    if (xfile_cat.is_debug()) {
      xfile_cat.debug()
        << "Ignoring animation set object of unknown type: "
        << obj->get_template_name() << "\n";
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_animation
//       Access: Private
//  Description: Converts the indicated Animation template object.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_animation(XFileDataNode *obj, XFileAnimationSet &animation_set) {
  // Within an Animation template, we expect to find a reference to a
  // frame, possibly an AnimationOptions object, and one or more
  // AnimationKey objects.

  // First, walk through the list of children, to find the one that is
  // the frame reference.  We need to know this up front so we know
  // which table we should be building up.
  string frame_name;
  bool got_frame_name = false;

  int num_objects = obj->get_num_objects();
  int i;
  for (i = 0; i < num_objects; i++) {
    XFileDataNode *child = obj->get_object(i);
    if (child->is_reference() && child->is_standard_object("Frame")) {
      frame_name = child->get_name();
      got_frame_name = true;
    }
  }

  if (!got_frame_name) {
    xfile_cat.error()
      << "Animation " << obj->get_name()
      << " includes no reference to a frame.\n";
    return false;
  }

  FrameData &table = animation_set.create_frame_data(frame_name);

  // Now go back again and get the actual data.
  for (i = 0; i < num_objects; i++) {
    if (!convert_animation_object(obj->get_object(i), frame_name, table)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_animation_object
//       Access: Private
//  Description: Converts the indicated object, a child of a
//               Animation.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_animation_object(XFileDataNode *obj, const string &joint_name,
                         XFileToEggConverter::FrameData &table) {
  if (obj->is_standard_object("AnimationOptions")) {
    // Quietly ignore AnimationOptions.

  } else if (obj->is_standard_object("Frame")) {
    // Quietly ignore frames, since we already got the frame name.

  } else if (obj->is_standard_object("AnimationKey")) {
    if (!convert_animation_key(obj, joint_name, table)) {
      return false;
    }

  } else {
    if (xfile_cat.is_debug()) {
      xfile_cat.debug()
        << "Ignoring animation object of unknown type: "
        << obj->get_template_name() << "\n";
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_animation_key
//       Access: Private
//  Description: Converts the indicated AnimationKey template object.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_animation_key(XFileDataNode *obj, const string &joint_name, 
                      XFileToEggConverter::FrameData &table) {
  int key_type = (*obj)["keyType"].i();
  
  const XFileDataObject &keys = (*obj)["keys"];

  for (int i = 0; i < keys.size(); i++) {
    // We ignore the time value.  It seems to be of questionable value
    // anyway, being of all sorts of crazy scales; and Panda doesn't
    // support timestamped keyframes anyway.  Assume the x file was
    // generated with one frame per frame of animation, and translate
    // accordingly.

    const XFileDataObject &values = keys[i]["tfkeys"]["values"];
    if (!set_animation_frame(joint_name, table, i, key_type, values)) {
      return false;
    }
  }

  return true;
}
    
////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::set_animation_frame
//       Access: Private
//  Description: Sets a single frame of the animation data.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
set_animation_frame(const string &joint_name, 
                    XFileToEggConverter::FrameData &table, int frame,
                    int key_type, const XFileDataObject &values) {
  if ((int)table._entries.size() <= frame) {
    nassertr((int)table._entries.size() == frame, false);
    table._entries.push_back(XFileAnimationSet::FrameEntry());
  }

  XFileAnimationSet::FrameEntry &frame_entry = table._entries[frame];

  // Now modify the last row in the table.
  switch (key_type) {
  case 0:
    // Key type 0: rotation.
    // This appears to be a quaternion.  Hope we get the coordinate
    // system right.
    if (values.size() != 4) {
      xfile_cat.error()
        << "Incorrect number of values in animation table: "
        << values.size() << " for rotation data.\n";
      return false;
    }
    frame_entry._rot.invert_from(LQuaterniond(values.vec4()));
    table._flags |= XFileAnimationSet::FDF_rot;
    break;

  case 1:
    if (values.size() != 3) {
      xfile_cat.error()
        << "Incorrect number of values in animation table: "
        << values.size() << " for scale data.\n";
      return false;
    }
    frame_entry._scale = values.vec3();
    table._flags |= XFileAnimationSet::FDF_scale;
    break;
    
  case 2:
    // Key type 2: position
    if (values.size() != 3) {
      xfile_cat.error()
        << "Incorrect number of values in animation table: "
        << values.size() << " for position data.\n";
      return false;
    }
    frame_entry._trans = values.vec3();
    table._flags |= XFileAnimationSet::FDF_trans;
    break;

    /*
  case 3:
    // Key type 3: ????
    break;
    */

  case 4:
    // Key type 4: full matrix
    if (values.size() != 16) {
      xfile_cat.error()
        << "Incorrect number of values in animation table: "
        << values.size() << " for matrix data.\n";
      return false;
    }
    frame_entry._mat = values.mat4();
    table._flags |= XFileAnimationSet::FDF_mat;
    break;

  default:
    xfile_cat.error()
      << "Unsupported key type " << key_type << " in animation table.\n";
    return false;
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_mesh
//       Access: Private
//  Description: Converts the indicated mesh to the appropriate egg
//               structures.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_mesh(XFileDataNode *obj, EggGroupNode *egg_parent) {
  XFileMesh *mesh = new XFileMesh(_egg_data->get_coordinate_system());
  mesh->set_name(obj->get_name());
  mesh->set_egg_parent(egg_parent);

  if (!mesh->fill_mesh(obj)) {
    delete mesh;
    return false;
  }

  _meshes.push_back(mesh);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::create_polygons
//       Access: Private
//  Description: Creates all the polygons associated with
//               previously-saved meshes.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
create_polygons() {
  bool okflag = true;

  Meshes::const_iterator mi;
  for (mi = _meshes.begin(); mi != _meshes.end(); ++mi) {
    if (!(*mi)->create_polygons(this)) {
      okflag = false;
    }
    delete (*mi);
  }
  _meshes.clear();

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::create_hierarchy
//       Access: Private
//  Description: Creates the animation table hierarchies for the
//               previously-saved animation sets.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
create_hierarchy() {
  bool okflag = true;

  AnimationSets::const_iterator asi;
  for (asi = _animation_sets.begin(); asi != _animation_sets.end(); ++asi) {
    if (_make_char) {
      if (!(*asi)->create_hierarchy(this)) {
        okflag = false;
      }
    }
    delete (*asi);
  }
  _animation_sets.clear();

  return okflag;
}
