// Filename: windowProperties.h
// Created by:  drose (13Aug02)
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

#ifndef WINDOWPROPERTIES_H
#define WINDOWPROPERTIES_H

#include "pandabase.h"
#include "filename.h"
#include "pnotify.h"
   
////////////////////////////////////////////////////////////////////
//       Class : WindowProperties
// Description : A container for the various kinds of properties we
//               might ask to have on a graphics window before we open
//               it.  This also serves to hold the current properties
//               for a window after it has been opened.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA WindowProperties {
PUBLISHED:


  enum ZOrder {
    Z_bottom,
    Z_normal,
    Z_top,
  };
  
  enum MouseMode {
    M_absolute,
    M_relative,
  };

  WindowProperties();
  INLINE WindowProperties(const WindowProperties &copy);
  void operator = (const WindowProperties &copy);
  INLINE ~WindowProperties();

  static WindowProperties get_default();
  static WindowProperties size(int x_size, int y_size);

  bool operator == (const WindowProperties &other) const;
  INLINE bool operator != (const WindowProperties &other) const;

  void clear();
  INLINE bool is_any_specified() const;
  
  INLINE void set_origin(int x_origin, int y_origin);
  INLINE int get_x_origin() const;
  INLINE int get_y_origin() const;
  INLINE bool has_origin() const;
  INLINE void clear_origin();

  INLINE void set_size(int x_size, int y_size);
  INLINE int get_x_size() const;
  INLINE int get_y_size() const;
  INLINE bool has_size() const;
  INLINE void clear_size();

  INLINE bool has_mouse_mode() const;
  INLINE void set_mouse_mode(MouseMode mode);
  INLINE MouseMode get_mouse_mode() const;
  INLINE void clear_mouse_mode();  

  INLINE void set_title(const string &title);
  INLINE const string &get_title() const;
  INLINE bool has_title() const;
  INLINE void clear_title();

  INLINE void set_undecorated(bool undecorated);
  INLINE bool get_undecorated() const;
  INLINE bool has_undecorated() const;
  INLINE void clear_undecorated();

  INLINE void set_fixed_size(bool fixed_size);
  INLINE bool get_fixed_size() const;
  INLINE bool has_fixed_size() const;
  INLINE void clear_fixed_size();

  INLINE void set_fullscreen(bool fullscreen);
  INLINE bool get_fullscreen() const;
  INLINE bool has_fullscreen() const;
  INLINE void clear_fullscreen();

  INLINE void set_foreground(bool foreground);
  INLINE bool get_foreground() const;
  INLINE bool has_foreground() const;
  INLINE void clear_foreground();

  INLINE void set_minimized(bool minimized);
  INLINE bool get_minimized() const;
  INLINE bool has_minimized() const;
  INLINE void clear_minimized();

  INLINE void set_raw_mice(bool raw_mice);
  INLINE bool get_raw_mice() const;
  INLINE bool has_raw_mice() const;
  INLINE void clear_raw_mice();

  INLINE void set_open(bool open);
  INLINE bool get_open() const;
  INLINE bool has_open() const;
  INLINE void clear_open();

  INLINE void set_cursor_hidden(bool cursor_hidden);
  INLINE bool get_cursor_hidden() const;
  INLINE bool has_cursor_hidden() const;
  INLINE void clear_cursor_hidden();

  INLINE void set_icon_filename(const Filename &icon_filename);
  INLINE const Filename &get_icon_filename() const;
  INLINE bool has_icon_filename() const;
  INLINE void clear_icon_filename();

  INLINE void set_cursor_filename(const Filename &cursor_filename);
  INLINE const Filename &get_cursor_filename() const;
  INLINE bool has_cursor_filename() const;
  INLINE void clear_cursor_filename();

  INLINE void set_z_order(ZOrder z_order);
  INLINE ZOrder get_z_order() const;
  INLINE bool has_z_order() const;
  INLINE void clear_z_order();


  INLINE void set_parent_window(size_t parent);
  INLINE size_t  get_parent_window() const;
  INLINE bool has_parent_window() const;
  INLINE void clear_parent_window();

  void add_properties(const WindowProperties &other);

  void output(ostream &out) const;

private:
  // This bitmask indicates which of the parameters in the properties
  // structure have been filled in by the user, and which remain
  // unspecified.
  enum Specified {
    S_origin           = 0x0001,
    S_size             = 0x0002,
    S_title            = 0x0004,
    S_undecorated      = 0x0008,
    S_fullscreen       = 0x0010,
    S_foreground       = 0x0020,
    S_minimized        = 0x0040,
    S_open             = 0x0080,
    S_cursor_hidden    = 0x0100,
    S_fixed_size       = 0x0200,
    S_z_order          = 0x0400,
    S_icon_filename    = 0x0800,
    S_cursor_filename  = 0x1000,
    S_mouse_mode       = 0x2000,
    S_parent_window    = 0x4000,
    S_raw_mice         = 0x8000,
  };

  // This bitmask represents the true/false settings for various
  // boolean flags (assuming the corresponding S_* bit has been set,
  // above).
  enum Flags {
    F_undecorated    = S_undecorated,
    F_fullscreen     = S_fullscreen,
    F_foreground     = S_foreground,
    F_minimized      = S_minimized,
    F_open           = S_open,
    F_cursor_hidden  = S_cursor_hidden,
    F_fixed_size     = S_fixed_size,
    F_raw_mice       = S_raw_mice,
  };

  int _specified;
  int _x_origin;
  int _y_origin;
  int _x_size;
  int _y_size;
  MouseMode _mouse_mode;
  string _title;
  Filename _cursor_filename;
  Filename _icon_filename;
  ZOrder _z_order;
  int _flags;
  size_t _parent_window;  // a HWND or WindowRef or .. what ever it is on X win...
};

EXPCL_PANDA ostream &
operator << (ostream &out, WindowProperties::ZOrder z_order);
EXPCL_PANDA istream &
operator >> (istream &in, WindowProperties::ZOrder &z_order);

EXPCL_PANDA ostream &
operator << (ostream &out, WindowProperties::MouseMode mode);
EXPCL_PANDA istream &
operator >> (istream &in, WindowProperties::MouseMode &mode);


INLINE ostream &operator << (ostream &out, const WindowProperties &properties);

#include "windowProperties.I"

#endif
