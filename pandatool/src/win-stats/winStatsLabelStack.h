// Filename: winStatsLabelStack.h
// Created by:  drose (07Jan04)
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

#ifndef WINSTATSLABELSTACK_H
#define WINSTATSLABELSTACK_H

#include "pandatoolbase.h"
#include "pvector.h"

#include <windows.h>

class WinStatsLabel;
class WinStatsMonitor;

////////////////////////////////////////////////////////////////////
//       Class : WinStatsLabelStack
// Description : A window that contains a stack of labels from bottom
//               to top.
////////////////////////////////////////////////////////////////////
class WinStatsLabelStack {
public:
  WinStatsLabelStack();
  ~WinStatsLabelStack();

  void setup(HWND parent_window);
  bool is_setup() const;
  void set_pos(int x, int y, int width, int height);

  int get_x() const;
  int get_y() const;
  int get_width() const;
  int get_height() const;
  int get_ideal_width() const;

  void clear_labels();
  void add_label(WinStatsMonitor *monitor, int collector_index);

private:
  void create_window(HWND parent_window);
  static void register_window_class(HINSTANCE application);

  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  LONG WINAPI window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  HWND _window;
  int _x;
  int _y;
  int _width;
  int _height;
  int _ideal_width;

  typedef pvector<WinStatsLabel *> Labels;
  Labels _labels;

  static bool _window_class_registered;
  static const char * const _window_class_name;
};

#endif

