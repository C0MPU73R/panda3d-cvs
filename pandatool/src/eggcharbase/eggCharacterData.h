// Filename: eggCharacterData.h
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGCHARACTERDATA_H
#define EGGCHARACTERDATA_H

#include <pandatoolbase.h>

#include <namable.h>

#include <map>

class EggCharacterCollection;
class EggJointData;
class EggSliderData;

////////////////////////////////////////////////////////////////////
// 	 Class : EggCharacterData
// Description : Represents a single character, as read and collected
//               from several models and animation files.  This
//               contains a hierarchy of EggJointData nodes
//               representing the skeleton, as well as a list of
//               EggSliderData nodes representing the morph channels
//               for the character.
//
//               This is very similar to the Character class from
//               Panda, in that it's capable of associating
//               skeleton-morph animation channels with models and
//               calculating the vertex position for each frame.  To
//               some degree, it duplicates the functionality of
//               Character.  However, it differs in one fundamental
//               principle: it is designed to be a non-real-time
//               operation, working directly on the Egg structures as
//               they are, instead of first boiling the Egg data into
//               native Panda Geom tables for real-time animation.
//               Because of this, it is (a) double-precision instead
//               of single precision, (b) capable of generating
//               modified Egg files, and (c) about a hundred times
//               slower than the Panda Character class.
////////////////////////////////////////////////////////////////////
class EggCharacterData : public Namable {
public:
  EggCharacterData(EggCharacterCollection *collection);
  virtual ~EggCharacterData();

  INLINE EggJointData *get_root_joint() const;

  EggSliderData *make_slider(const string &name);

  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  EggCharacterCollection *_collection;
  EggJointData *_root_joint;

  typedef map<string, EggSliderData *> Sliders;
  Sliders _sliders;
};

#include "eggCharacterData.I"

#endif


