// Filename: animBundleNode.h
// Created by:  drose (06Mar02)
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

#ifndef ANIMBUNDLENODE_H
#define ANIMBUNDLENODE_H

#include "pandabase.h"

#include "animBundle.h"

#include "pandaNode.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//       Class : AnimBundleNode
// Description : This is a node that contains a pointer to an
//               AnimBundle.  Like AnimBundleNode, it exists solely to
//               make it easy to store AnimBundles in the scene graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AnimBundleNode : public PandaNode {
public:
  INLINE AnimBundleNode(const string &name, AnimBundle *bundle);

protected:
  INLINE AnimBundleNode();
  INLINE AnimBundleNode(const AnimBundleNode &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool safe_to_flatten() const;

PUBLISHED:
  INLINE AnimBundle *get_bundle() const;

private:
  PT(AnimBundle) _bundle;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "AnimBundleNode",
                  PandaNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animBundleNode.I"

#endif
