// Filename: config_dtoolutil.cxx
// Created by:  drose (17Nov06)
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

#include "config_dtoolutil.h"

#include "filename.h"
#include "pandaSystem.h"

////////////////////////////////////////////////////////////////////
//     Function: init_libdtoolutil
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libdtoolutil() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  Filename::init_type();
  PandaSystem::init_type();
}

class InitDtoolutil {
public:
  InitDtoolutil() {
    init_libdtoolutil();
  }
};

static InitDtoolutil _init;
