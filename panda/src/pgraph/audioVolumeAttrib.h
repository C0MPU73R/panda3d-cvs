// Filename: audioVolumeAttrib.h
// Created by:  darren (15Dec06)
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

#ifndef AUDIOVOLUMEATTRIB_H
#define AUDIOVOLUMEATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "luse.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : AudioVolumeAttrib
// Description : Applies a scale to audio volume for positional sounds
//               in the scene graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH AudioVolumeAttrib : public RenderAttrib {
protected:
  AudioVolumeAttrib(bool off, float volume);
  INLINE AudioVolumeAttrib(const AudioVolumeAttrib &copy);

PUBLISHED:
  static CPT(RenderAttrib) make_identity();
  static CPT(RenderAttrib) make(float volume);
  static CPT(RenderAttrib) make_off();
  static CPT(RenderAttrib) make_default();

  INLINE bool is_off() const;
  INLINE bool has_volume() const;
  INLINE float get_volume() const;
  CPT(RenderAttrib) set_volume(float volume) const;

public:
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;

private:
  bool _off;
  bool _has_volume;
  float _volume;
  static CPT(RenderAttrib) _identity_attrib;

PUBLISHED:
  static int get_class_slot() {
    return _attrib_slot;
  }
  virtual int get_slot() const {
    return get_class_slot();
  }

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "AudioVolumeAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100, make_default);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "audioVolumeAttrib.I"

#endif
