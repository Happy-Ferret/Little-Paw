// Filename: driveInterface.h
// Created by:  drose (17Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef DRIVEINTERFACE_H
#define DRIVEINTERFACE_H

#include <pandabase.h>

#include <dataNode.h>
#include <vec3DataTransition.h>
#include <vec3DataAttribute.h>
#include <matrixDataTransition.h>
#include <matrixDataAttribute.h>
#include <nodeAttributes.h>
#include <modifierButtons.h>
#include <luse.h>
#include <lmatrix.h>


////////////////////////////////////////////////////////////////////
//       Class : DriveInterface
// Description : This is a TFormer, similar to Trackball, that moves
//               around a transform matrix in response to mouse input.
//               The basic motion is on a horizontal plane, as if
//               driving a vehicle.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DriveInterface : public DataNode {
PUBLISHED:
  DriveInterface(const string &name = "");
  ~DriveInterface();

  INLINE void set_forward_speed(float speed);
  INLINE float get_forward_speed() const;
  INLINE void set_reverse_speed(float speed);
  INLINE float get_reverse_speed() const;
  INLINE void set_rotate_speed(float speed);
  INLINE float get_rotate_speed() const;
  INLINE void set_vertical_dead_zone(float zone);
  INLINE float get_vertical_dead_zone() const;
  INLINE void set_horizontal_dead_zone(float zone);
  INLINE float get_horizontal_dead_zone() const;

  INLINE void set_vertical_ramp_up_time(float ramp_up_time);
  INLINE float get_vertical_ramp_up_time() const;
  INLINE void set_vertical_ramp_down_time(float ramp_down_time);
  INLINE float get_vertical_ramp_down_time() const;
  INLINE void set_horizontal_ramp_up_time(float ramp_up_time);
  INLINE float get_horizontal_ramp_up_time() const;
  INLINE void set_horizontal_ramp_down_time(float ramp_down_time);
  INLINE float get_horizontal_ramp_down_time() const;

  INLINE float get_speed() const;
  INLINE float get_rot_speed() const;

  void reset();

  /// **** Translation ****

  INLINE const LPoint3f &get_pos() const;
  INLINE float get_x() const;
  INLINE float get_y() const;
  INLINE float get_z() const;
  INLINE void set_pos(const LVecBase3f &vec);
  INLINE void set_pos(float x, float y, float z);
  INLINE void set_x(float x);
  INLINE void set_y(float y);
  INLINE void set_z(float z);
 
  /// **** Rotation ****
 
  INLINE const LVecBase3f &get_hpr() const;
  INLINE float get_h() const;
  INLINE float get_p() const;
  INLINE float get_r() const;
  INLINE void set_hpr(const LVecBase3f &hpr);
  INLINE void set_hpr(float h, float p, float r);
  INLINE void set_h(float h);
  INLINE void set_p(float p);
  INLINE void set_r(float r);

  void set_force_roll(float force_roll);
  INLINE bool is_force_roll() const;
  INLINE void clear_force_roll();

  INLINE void set_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_coordinate_system() const;

  INLINE void set_ignore_mouse(bool ignore_mouse);
  INLINE bool get_ignore_mouse() const;

  void set_mat(const LMatrix4f &mat);
  const LMatrix4f &get_mat() const;

  void force_dgraph();

private:
  void apply(double x, double y, bool any_button);

  void reextract();
  void recompute();

  float _forward_speed;  // units / sec, mouse all the way up
  float _reverse_speed;  // units / sec, mouse all the way down
  float _rotate_speed;   // degrees / sec, mouse all the way over
  float _vertical_dead_zone;    // fraction of window size
  float _horizontal_dead_zone;  // fraction of window size
  float _vertical_center;    // window units, 0 = center, -1 = bottom, 1 = top
  float _horizontal_center;  // window units, 0 = center, -1 = left, 1 = right

  // The time it takes to ramp up to full speed from a stop (or return
  // to a stop from full speed) when using the keyboard.
  float _vertical_ramp_up_time;
  float _vertical_ramp_down_time;
  float _horizontal_ramp_up_time;
  float _horizontal_ramp_down_time;

  float _speed; // instantaneous units / sec
  float _rot_speed; // instantaneous rotational units / sec

  LPoint3f _xyz;
  LVecBase3f _hpr;
  LMatrix4f _mat;
  float _force_roll;
  bool _is_force_roll;
  CoordinateSystem _cs;
  bool _ignore_mouse;

  // Remember which mouse buttons are being held down.
  ModifierButtons _mods;

  // Remember which arrow keys are being held down and which aren't,
  // and at what point they last changed state.
  class KeyHeld {
  public:
    KeyHeld();
    float get_effect(float ramp_up_time, float ramp_down_time);
    void set_key(bool down);
    void clear();
    bool operator < (const KeyHeld &other) const;
  
    float _effect;
    bool _down;
    double _changed_time;
    float _effect_at_change;
  };
  KeyHeld _up_arrow, _down_arrow;
  KeyHeld _left_arrow, _right_arrow;

////////////////////////////////////////////////////////////////////
// From parent class DataNode
////////////////////////////////////////////////////////////////////
public:

  virtual void
  transmit_data(NodeAttributes &data);

  NodeAttributes _attrib;
  PT(MatrixDataAttribute) _transform;

  // inputs
  static TypeHandle _xyz_type;
  static TypeHandle _button_events_type;

  // outputs
  static TypeHandle _transform_type;


public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "driveInterface.I"

#endif
