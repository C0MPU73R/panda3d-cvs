// Filename: trackerNode.h
// Created by:  jason (07Aug00)
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

#ifndef TRACKERNODE_H
#define TRACKERNODE_H

#include <pandabase.h>

#include "clientBase.h"
#include "trackerData.h"
#include "clientTrackerDevice.h"

#include <dataNode.h>
#include <matrixDataTransition.h>
#include <matrixDataTransition.h>
#include <allTransitionsWrapper.h>
#include <luse.h>
#include <lmatrix.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
//       Class : TrackerNode
// Description : This is the primary interface to a Tracker object
//               associated with a ClientBase.  It reads the position
//               and orientation information from the tracker and
//               makes it available as a transformation on the data
//               graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TrackerNode : public DataNode {
PUBLISHED:
  TrackerNode(ClientBase *client, const string &device_name);
  virtual ~TrackerNode();

  INLINE bool is_valid() const;

  INLINE const LPoint3f &get_pos() const;
  INLINE const LOrientationf &get_orient() const;
  INLINE const LMatrix4f &get_transform() const;

  INLINE void set_tracker_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_tracker_coordinate_system() const;

  INLINE void set_graph_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_graph_coordinate_system() const;

////////////////////////////////////////////////////////////////////
// From parent class DataNode
////////////////////////////////////////////////////////////////////
public:
  virtual void
  transmit_data(AllTransitionsWrapper &data);

  AllTransitionsWrapper _attrib;
  PT(MatrixDataTransition) _transform_attrib;

  // outputs
  static TypeHandle _transform_type;

private:
  PT(ClientTrackerDevice) _tracker;
  TrackerData _data;
  LMatrix4f _transform;
  CoordinateSystem _tracker_cs, _graph_cs;

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

#include "trackerNode.I"

#endif
