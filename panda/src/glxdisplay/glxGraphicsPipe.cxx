// Filename: glxGraphicsPipe.cxx
// Created by:  mike (09Jan97)
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

#include "glxGraphicsPipe.h"
#include "glxGraphicsWindow.h"
#include "glxGraphicsBuffer.h"
#include "glxGraphicsStateGuardian.h"
#include "config_glxdisplay.h"
#include "frameBufferProperties.h"

TypeHandle glxGraphicsPipe::_type_handle;

bool glxGraphicsPipe::_error_handlers_installed = false;
glxGraphicsPipe::ErrorHandlerFunc *glxGraphicsPipe::_prev_error_handler;
glxGraphicsPipe::IOErrorHandlerFunc *glxGraphicsPipe::_prev_io_error_handler;

ReMutex glxGraphicsPipe::_x_mutex;

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
glxGraphicsPipe::
glxGraphicsPipe(const string &display) {
  string display_spec = display;
  if (display_spec.empty()) {
    display_spec = display_cfg;
  }
  if (display_spec.empty()) {
    display_spec = ExecutionEnvironment::get_environment_variable("DISPLAY");
  }
  if (display_spec.empty()) {
    display_spec = ":0.0";
  }

  // The X docs say we should do this to get international character
  // support from the keyboard.
  setlocale(LC_ALL, "");

  // But it's important that we use the "C" locale for numeric
  // formatting, since all of the internal Panda code assumes this--we
  // need a decimal point to mean a decimal point.
  setlocale(LC_NUMERIC, "C");

  _is_valid = false;
  _supported_types = OT_window | OT_buffer | OT_texture_buffer;
  _display = NULL;
  _screen = 0;
  _root = (Window)NULL;
  _im = (XIM)NULL;
  _hidden_cursor = None;

  install_error_handlers();

  _display = XOpenDisplay(display_spec.c_str());
  if (!_display) {
    glxdisplay_cat.error()
      << "Could not open display \"" << display_spec << "\".\n";
    return;
  }

  if (!XSupportsLocale()) {
    glxdisplay_cat.warning()
      << "X does not support locale " << setlocale(LC_ALL, NULL) << "\n";
  }
  XSetLocaleModifiers("");

  int errorBase, eventBase;
  if (!glXQueryExtension(_display, &errorBase, &eventBase)) {
    glxdisplay_cat.error()
      << "OpenGL GLX extension not supported on display \"" << display_spec
      << "\".\n";
    return;
  }

  _screen = DefaultScreen(_display);
  _root = RootWindow(_display, _screen);
  _display_width = DisplayWidth(_display, _screen);
  _display_height = DisplayHeight(_display, _screen);
  _is_valid = true;

  // Connect to an input method for supporting international text
  // entry.
  _im = XOpenIM(_display, NULL, NULL, NULL);
  if (_im == (XIM)NULL) {
    glxdisplay_cat.warning()
      << "Couldn't open input method.\n";
  }

  // What styles does the current input method support?
  /*
  XIMStyles *im_supported_styles;
  XGetIMValues(_im, XNQueryInputStyle, &im_supported_styles, NULL);

  for (int i = 0; i < im_supported_styles->count_styles; i++) {
    XIMStyle style = im_supported_styles->supported_styles[i];
    cerr << "style " << i << ". " << hex << style << dec << "\n";
  }

  XFree(im_supported_styles);
  */

  // Get some X atom numbers.
  _wm_delete_window = XInternAtom(_display, "WM_DELETE_WINDOW", false);
  _net_wm_window_type = XInternAtom(_display, "_NET_WM_WINDOW_TYPE", false);
  _net_wm_window_type_splash = XInternAtom(_display, "_NET_WM_WINDOW_TYPE_SPLASH", false);
  _net_wm_window_type_fullscreen = XInternAtom(_display, "_NET_WM_WINDOW_TYPE_FULLSCREEN", false);
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
glxGraphicsPipe::
~glxGraphicsPipe() {
  release_hidden_cursor();
  if (_im) {
    XCloseIM(_im);
  }
  if (_display) {
    XCloseDisplay(_display);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string glxGraphicsPipe::
get_interface_name() const {
  return "OpenGL";
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               glxGraphicsPipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) glxGraphicsPipe::
pipe_constructor() {
  return new glxGraphicsPipe;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::make_output
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsOutput) glxGraphicsPipe::
make_output(const string &name,
            const FrameBufferProperties &properties,
            int x_size, int y_size, int flags,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {
  
  if (!_is_valid) {
    return NULL;
  }

  glxGraphicsStateGuardian *glxgsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(glxgsg, gsg, NULL);
  }
  
  // First thing to try: a glxGraphicsWindow

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }
    return new glxGraphicsWindow(this, name, properties,
                                 x_size, y_size, flags, gsg, host);
  }
  
  // Second thing to try: a GLGraphicsBuffer

  if (retry == 1) {
    if ((!support_render_texture)||
	//        (!gl_support_fbo)||
        (host==0)||
        ((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)) {
      return NULL;
    }
    // Early failure - if we are sure that this buffer WONT
    // meet specs, we can bail out early.
    if ((flags & BF_fb_props_optional)==0) {
      if ((properties.get_indexed_color() > 0)||
          (properties.get_back_buffers() > 0)||
          (properties.get_accum_bits() > 0)||
          (properties.get_multisamples() > 0)) {
        return NULL;
      }
    }
    // Early success - if we are sure that this buffer WILL
    // meet specs, we can precertify it.
    if ((glxgsg != 0) &&
        (glxgsg->is_valid()) &&
        (!glxgsg->needs_reset()) &&
        (glxgsg->_supports_framebuffer_object) &&
        (glxgsg->_glDrawBuffers != 0)&&
        (properties.is_basic())) {
      precertify = true;
    }
    return new GLGraphicsBuffer(this, name, properties,
                                x_size, y_size, flags, gsg, host);
  }
  
#ifdef HAVE_GLXFBCONFIG
  // Third thing to try: a glxGraphicsBuffer
  
  if (retry == 2) {
    if ((!support_render_texture)||
        ((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }
    return new glxGraphicsBuffer(this, name, properties,
                                 x_size, y_size, flags, gsg, host);
  }
#endif  // HAVE_GLXFBCONFIG
  
  // Nothing else left to try.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::make_hidden_cursor
//       Access: Private
//  Description: Called once to make an invisible Cursor for return
//               from get_hidden_cursor().
////////////////////////////////////////////////////////////////////
void glxGraphicsPipe::
make_hidden_cursor() {
  nassertv(_hidden_cursor == None);

  unsigned int x_size, y_size;
  XQueryBestCursor(_display, _root, 1, 1, &x_size, &y_size);

  Pixmap empty = XCreatePixmap(_display, _root, x_size, y_size, 1);

  XColor black;
  memset(&black, 0, sizeof(black));

  _hidden_cursor = XCreatePixmapCursor(_display, empty, empty, 
                                       &black, &black, x_size, y_size);
  XFreePixmap(_display, empty);
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::release_hidden_cursor
//       Access: Private
//  Description: Called once to release the invisible cursor created
//               by make_hidden_cursor().
////////////////////////////////////////////////////////////////////
void glxGraphicsPipe::
release_hidden_cursor() {
  if (_hidden_cursor != None) {
    XFreeCursor(_display, _hidden_cursor);
    _hidden_cursor = None;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::install_error_handlers
//       Access: Private, Static
//  Description: Installs new Xlib error handler functions if this is
//               the first time this function has been called.  These
//               error handler functions will attempt to reduce Xlib's
//               annoying tendency to shut down the client at the
//               first error.  Unfortunately, it is difficult to play
//               nice with the client if it has already installed its
//               own error handlers.
////////////////////////////////////////////////////////////////////
void glxGraphicsPipe::
install_error_handlers() {
  if (_error_handlers_installed) {
    return;
  }

  _prev_error_handler = (ErrorHandlerFunc *)XSetErrorHandler(error_handler);
  _prev_io_error_handler = (IOErrorHandlerFunc *)XSetIOErrorHandler(io_error_handler);
  _error_handlers_installed = true;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::error_handler
//       Access: Private, Static
//  Description: This function is installed as the error handler for a
//               non-fatal Xlib error.
////////////////////////////////////////////////////////////////////
int glxGraphicsPipe::
error_handler(Display *display, XErrorEvent *error) {
  static const int msg_len = 80;
  char msg[msg_len];
  XGetErrorText(display, error->error_code, msg, msg_len);
  glxdisplay_cat.error()
    << msg << "\n";

  if (glx_error_abort) {
    abort();
  }

  // We return to allow the application to continue running, unlike
  // the default X error handler which exits.
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::io_error_handler
//       Access: Private, Static
//  Description: This function is installed as the error handler for a
//               fatal Xlib error.
////////////////////////////////////////////////////////////////////
int glxGraphicsPipe::
io_error_handler(Display *display) {
  glxdisplay_cat.fatal()
    << "X fatal error on display " << (void *)display << "\n";

  // Unfortunately, we can't continue from this function, even if we
  // promise never to use X again.  We're supposed to terminate
  // without returning, and if we do return, the caller will exit
  // anyway.  Sigh.  Very poor design on X's part.
  return 0;
}
