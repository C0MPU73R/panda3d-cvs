// Filename: winGraphicsWindow.h
// Created by:  drose (20Dec02)
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

#ifndef WINGRAPHICSWINDOW_H
#define WINGRAPHICSWINDOW_H

// include win32 defns for everything up to WinServer2003, and assume I'm smart
// enough to use GetProcAddress for backward compat on w95/w98 for
// newer fns
#define _WIN32_WINNT 0x0502

// Jesse thinks that this is supposed to say WIN32_LEAN_AND_MEAN, but he
// doesn't want to fix what isn't broken.
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN

#include "pandabase.h"
#include "graphicsWindow.h"

class WinGraphicsPipe;

////////////////////////////////////////////////////////////////////
//       Class : WinGraphicsWindow
// Description : An abstract base class for glGraphicsWindow and
//               dxGraphicsWindow (and, in general, graphics windows
//               that interface with the Microsoft Windows API).
//
//               This class includes all the code for manipulating
//               windows themselves: opening them, closing them,
//               responding to user keyboard and mouse input, and so
//               on.  It does not make any 3-D rendering calls into
//               the window; that is the province of the
//               GraphicsStateGuardian.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAWIN WinGraphicsWindow : public GraphicsWindow {
public:
  WinGraphicsWindow(GraphicsPipe *pipe, GraphicsStateGuardian *gsg);
  virtual ~WinGraphicsWindow();

  virtual void begin_flip();

  virtual void process_events();
  virtual void set_properties_now(WindowProperties &properties);
  virtual LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  virtual bool handle_mouse_motion(int x, int y);
  virtual void handle_mouse_exit(void);


protected:
  virtual void close_window();
  virtual bool open_window();
  virtual void fullscreen_minimized(WindowProperties &properties);
  virtual void fullscreen_restored(WindowProperties &properties);

  virtual bool do_reshape_request(int x_origin, int y_origin, 
                                  int x_size, int y_size);

  virtual void handle_reshape();
  virtual bool do_fullscreen_resize(int x_size, int y_size);

  virtual void reconsider_fullscreen_size(DWORD &x_size, DWORD &y_size, 
                                          DWORD &bitdepth);

  virtual void support_overlay_window(bool flag);

private:
  bool open_fullscreen_window();
  bool open_regular_window();
  void track_mouse_leaving(HWND hwnd);

  static void process_1_event();

  INLINE void handle_keypress(ButtonHandle key, int x, int y);
  INLINE void handle_keyresume(ButtonHandle key);
  INLINE void handle_keyrelease(ButtonHandle key);
  ButtonHandle lookup_key(WPARAM wparam) const;
  INLINE int translate_mouse(int pos) const;
  INLINE void set_cursor_in_window();
  INLINE void set_cursor_out_of_window();

  void resend_lost_keypresses();
  static void update_cursor_window(WinGraphicsWindow *to_window);

  static void register_window_class();
  static bool find_acceptable_display_mode(DWORD dwWidth, DWORD dwHeight,
                                           DWORD bpp, DEVMODE &dm);
  static void show_error_message(DWORD message_id = 0);

protected:
  HWND _hWnd;

private:
  bool _ime_open;
  bool _ime_active;
  bool _ime_composition_w;
  bool _tracking_mouse_leaving;
  bool _maximized;
  bool _bCursor_in_WindowClientArea;
  DEVMODE _fullscreen_display_mode;

  // This is used to remember the state of the keyboard when keyboard
  // focus is lost.
  enum { num_virtual_keys = 256 };
  // You might be wondering why the above is an enum. Originally the line
  // read "static const int num_virtual_keys = 256"
  // but in trying to support the MSVC6 compiler, we found that you
  // were not allowed to define the value of a const within a class like
  // that. Defining the value outside the class helps, but then we can't
  // use the value to define the length of the _keyboard_state array, and
  // also it creates multiply defined symbol errors when you link, because
  // other files include this header file. This enum is a clever solution
  // to work around the problem.

  BYTE _keyboard_state[num_virtual_keys];
  bool _lost_keypresses;

protected:
  static bool _loaded_custom_cursor;
  static HCURSOR _mouse_cursor;
  static const char * const _window_class_name;
  static bool _window_class_registered;

private:
  // We need this map to support per-window calls to window_proc().
  typedef map<HWND, WinGraphicsWindow *> WindowHandles;
  static WindowHandles _window_handles;

  // And we need a static pointer to the current WinGraphicsWindow we
  // are creating at the moment, since CreateWindow() starts
  // generating window events before it gives us the window handle.
  static WinGraphicsWindow *_creating_window;

  // This tracks the current GraphicsWindow whose client area contains
  // the mouse.  There will only be one of these at a time, and
  // storing the pointer here allows us to handle ambiguities in the
  // order in which messages are passed from Windows to the various
  // windows we manage.  This pointer is used by
  // set_cursor_in_window() to determine when it is time to call
  // update_cursor() to hide the cursor (or do other related
  // operations).
  static WinGraphicsWindow *_cursor_window;
  static bool _cursor_hidden;
  static bool _got_saved_params;
  static int _saved_mouse_trails;
  static BOOL _saved_cursor_shadow;
  static BOOL _saved_mouse_vanish;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "WinGraphicsWindow",
                  GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#define PRINT_LAST_ERROR 0
extern EXPCL_PANDAWIN void PrintErrorMessage(DWORD msgID);
extern EXPCL_PANDAWIN void ClearToBlack(HWND hWnd, const WindowProperties &props);
extern EXPCL_PANDAWIN void get_client_rect_screen(HWND hwnd, RECT *view_rect);

#include "winGraphicsWindow.I"

#endif
