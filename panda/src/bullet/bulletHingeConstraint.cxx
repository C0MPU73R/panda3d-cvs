// Filename: bulletHingeConstraint.cxx
// Created by:  enn0x (01Mar10)
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

#include "bulletHingeConstraint.h"
#include "bulletRigidBodyNode.h"

#include "deg_2_rad.h"

TypeHandle BulletHingeConstraint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::Constructor
//       Access: Published
//  Description: Creates a hinge constraint in the same way as the
//               other constructor, but uses the world as second
//               body so that node_a is fixed to some point in
//               mid-air for example.
////////////////////////////////////////////////////////////////////
BulletHingeConstraint::
BulletHingeConstraint(const BulletRigidBodyNode *node_a, 
                      const LPoint3f &pivot_a,
                      const LVector3f &axis_a,
                      bool use_frame_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btVector3 pos_a = LVecBase3f_to_btVector3(pivot_a);
  btVector3 vec_a = LVecBase3f_to_btVector3(axis_a);

  _constraint = new btHingeConstraint(*ptr_a, pos_a, vec_a, use_frame_a);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::Constructor
//       Access: Published
//  Description: Creates a hinge connecting node_a to node_b. The
//               pivot point is the point at which the body is fixed
//               to the constraint. In other words: It specifies
//               where on each body the rotation axis should be. This
//               axis is specified using axis_a and axis_b.
//               Remember, everything is specified in the bodies own
//               coordinate system!
////////////////////////////////////////////////////////////////////
BulletHingeConstraint::
BulletHingeConstraint(const BulletRigidBodyNode *node_a,
                      const BulletRigidBodyNode *node_b,
                      const LPoint3f &pivot_a,
                      const LPoint3f &pivot_b,
                      const LVector3f &axis_a,
                      const LVector3f &axis_b,
                      bool use_frame_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btVector3 pos_a = LVecBase3f_to_btVector3(pivot_a);
  btVector3 vec_a = LVecBase3f_to_btVector3(axis_a);

  btRigidBody *ptr_b = btRigidBody::upcast(node_b->get_object());
  btVector3 pos_b = LVecBase3f_to_btVector3(pivot_b);
  btVector3 vec_b = LVecBase3f_to_btVector3(axis_b);

  _constraint = new btHingeConstraint(*ptr_a, *ptr_b, pos_a, pos_b, vec_a, vec_b, use_frame_a);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btTypedConstraint *BulletHingeConstraint::
ptr() const {

  return _constraint;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::set_angular_only
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletHingeConstraint::
set_angular_only(bool value) {

  return _constraint->setAngularOnly(value);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::get_angular_only
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletHingeConstraint::
get_angular_only() const {

  return _constraint->getAngularOnly();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::set_limit
//       Access: Published
//  Description: Sets the lower and upper rotational limits in
//               degrees.
////////////////////////////////////////////////////////////////////
void BulletHingeConstraint::
set_limit(float low, float high, float softness, float bias, float relaxation) {

  low  = deg_2_rad(low);
  high = deg_2_rad(high);

  _constraint->setLimit(low, high, softness, bias, relaxation);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::set_axis
//       Access: Published
//  Description: Sets the hinge's rotation axis in world
//               coordinates.
////////////////////////////////////////////////////////////////////
void BulletHingeConstraint::
set_axis(const LVector3f &axis) {

  nassertv(!axis.is_nan());

  btVector3 v = LVecBase3f_to_btVector3(axis);
  _constraint->setAxis(v);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::get_lower_limit
//       Access: Published
//  Description: Returns the lower angular limit in degrees.
////////////////////////////////////////////////////////////////////
float BulletHingeConstraint::
get_lower_limit() const {

  return rad_2_deg(_constraint->getLowerLimit());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::get_upper_limit
//       Access: Published
//  Description: Returns the upper angular limit in degrees.
////////////////////////////////////////////////////////////////////
float BulletHingeConstraint::
get_upper_limit() const {

  return rad_2_deg(_constraint->getUpperLimit());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::get_hinge_angle
//       Access: Published
//  Description: Returns the angle between node_a and node_b in
//               degrees.
////////////////////////////////////////////////////////////////////
float BulletHingeConstraint::
get_hinge_angle() {

  return rad_2_deg(_constraint->getHingeAngle());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::enable_angular_motor
//       Access: Published
//  Description: Applies an impulse to the constraint so that the
//               angle changes at target_velocity (probably
//               degrees/second) where max_impulse is the maximum
//               impulse that is used for achieving the specified
//               velocity.
////////////////////////////////////////////////////////////////////
void BulletHingeConstraint::
enable_angular_motor(bool enable, float target_velocity, float max_impulse) {

  _constraint->enableAngularMotor(enable, target_velocity, max_impulse);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::enable_motor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletHingeConstraint::
enable_motor(bool enable) {

  _constraint->enableMotor(enable);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::set_max_motor_impulse
//       Access: Published
//  Description: Sets the maximum impulse used to achieve the
//               velocity set in enable_angular_motor.
////////////////////////////////////////////////////////////////////
void BulletHingeConstraint::
set_max_motor_impulse(float max_impulse) {

  _constraint->setMaxMotorImpulse(max_impulse);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::set_motor_target
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletHingeConstraint::
set_motor_target(const LQuaternionf &quat, float dt) {

  _constraint->setMotorTarget(LQuaternionf_to_btQuat(quat), dt);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHingeConstraint::set_motor_target
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletHingeConstraint::
set_motor_target(float target_angle, float dt) {

  _constraint->setMotorTarget(target_angle, dt);
}

