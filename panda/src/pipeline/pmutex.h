// Filename: pmutex.h
// Created by:  cary (16Sep98)
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

#ifndef PMUTEX_H
#define PMUTEX_H

#include "pandabase.h"
#include "mutexDebug.h"
#include "mutexDirect.h"

////////////////////////////////////////////////////////////////////
//       Class : Mutex
// Description : A standard mutex, or mutual exclusion lock.  Only one
//               thread can hold ("lock") a mutex at any given time;
//               other threads trying to grab the mutex will block
//               until the holding thread releases it.
//
//               The standard mutex is not reentrant: a thread may not
//               attempt to lock it twice.  Although this may happen
//               to work on some platforms (e.g. Win32), it will not
//               work on all platforms; on some platforms, a thread
//               can deadlock itself by attempting to lock the same
//               mutex twice.  If your code requires a reentrant
//               mutex, use the ReMutex class instead.
//
//               This class inherits its implementation either from
//               MutexDebug or MutexDirect, depending on the
//               definition of DEBUG_THREADS.
////////////////////////////////////////////////////////////////////
#ifdef DEBUG_THREADS
class EXPCL_PANDA Mutex : public MutexDebug
#else
class EXPCL_PANDA Mutex : public MutexDirect
#endif  // DEBUG_THREADS
{
public:
  INLINE Mutex();
  INLINE Mutex(const char *name);
  INLINE Mutex(const string &name);
  INLINE ~Mutex();
private:
  INLINE Mutex(const Mutex &copy);
  INLINE void operator = (const Mutex &copy);

public:
  // This is a global mutex set aside for the purpose of protecting
  // Notify messages from being interleaved between threads.
  static Mutex _notify_mutex;
};

#include "pmutex.I"

#endif
