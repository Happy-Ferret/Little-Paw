// Filename: displayInformation.h
// Created by:  aignacio (17Jan07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2007, Disney Enterprises, Inc.  All rights 
// reserved.
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

#ifndef DISPLAYINFORMATION_H
#define DISPLAYINFORMATION_H

#include "typedef.h"

typedef struct {
  int width;
  int height;
  int bits_per_pixel;
  int refresh_rate;
  int fullscreen_only;
}
DisplayMode;

////////////////////////////////////////////////////////////////////
//       Class : DisplayInformation
// Description : This class contains various display information.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA_DISPLAY DisplayInformation {

PUBLISHED:

  enum DetectionState {
    DS_unknown,  
    DS_success,  

    DS_direct_3d_create_error,  
    DS_create_window_error,
    DS_create_device_error,
  };

  ~DisplayInformation();
  DisplayInformation();

  int get_display_state();

  int get_maximum_window_width();
  int get_maximum_window_height();
  int get_window_bits_per_pixel();

  int get_total_display_modes();
  int get_display_mode_width(int display_index);
  int get_display_mode_height(int display_index);
  int get_display_mode_bits_per_pixel(int display_index);
  int get_display_mode_refresh_rate(int display_index);
  int get_display_mode_fullscreen_only(int display_index);

  int get_shader_model();
  int get_video_memory();
  int get_texture_memory();

  void update_memory_information();
  PN_uint64 get_physical_memory();
  PN_uint64 get_available_physical_memory();
  PN_uint64 get_page_file_size();
  PN_uint64 get_available_page_file_size();
  PN_uint64 get_process_virtual_memory();
  PN_uint64 get_available_process_virtual_memory();
  int get_memory_load();
  PN_uint64 get_page_fault_count();
  PN_uint64 get_process_memory();
  PN_uint64 get_peak_process_memory();
  PN_uint64 get_page_file_usage();
  PN_uint64 get_peak_page_file_usage();

  int get_vendor_id();
  int get_device_id();
  
public:
  DetectionState _state;
  int _get_adapter_display_mode_state;
  int _get_device_caps_state;
  int _maximum_window_width;
  int _maximum_window_height;
  int _window_bits_per_pixel;
  int _total_display_modes;  
  DisplayMode *_display_mode_array;
  int _shader_model;
  int _video_memory;
  int _texture_memory;

  PN_uint64 _physical_memory;
  PN_uint64 _available_physical_memory;
  PN_uint64 _page_file_size;
  PN_uint64 _available_page_file_size;
  PN_uint64 _process_virtual_memory;
  PN_uint64 _available_process_virtual_memory;

  PN_uint64 _page_fault_count;
  PN_uint64 _process_memory;
  PN_uint64 _peak_process_memory;
  PN_uint64 _page_file_usage;
  PN_uint64 _peak_page_file_usage;
  
  int _memory_load;

  int _vendor_id;
  int _device_id;
  
  void (*_get_memory_information_function) (DisplayInformation *display_information);
};

#endif
