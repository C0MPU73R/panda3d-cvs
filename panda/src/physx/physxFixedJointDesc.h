// Filename: physxFixedJointDesc.h
// Created by:  enn0x (28Sep09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PHYSXFIXEDJOINTDESC_H
#define PHYSXFIXEDJOINTDESC_H

#include "pandabase.h"

#include "physxJointDesc.h"

#include "NoMinMax.h"
#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxFixedJointDesc
// Description : Descriptor class for fixed joint. A fixed joint
//               permits no relative movement between two bodies, 
//               i. e. the bodies are glued together. 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxFixedJointDesc : public PhysxJointDesc {

PUBLISHED:
  INLINE PhysxFixedJointDesc();
  INLINE ~PhysxFixedJointDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

public:
  NxJointDesc *ptr() const { return (NxJointDesc *)&_desc; };

private:
  NxFixedJointDesc _desc;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxJointDesc::init_type();
    register_type(_type_handle, "PhysxFixedJointDesc", 
                  PhysxJointDesc::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "physxFixedJointDesc.I"

#endif // PHYSXFIXEDJOINTDESC_H
