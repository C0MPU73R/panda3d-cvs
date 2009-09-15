// Filename: interrogateType.h
// Created by:  drose (31Jul00)
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

#ifndef INTERROGATETYPE_H
#define INTERROGATETYPE_H

#include "dtoolbase.h"

#include "interrogateComponent.h"

#include <vector>

class IndexRemapper;
class CPPType;
class CPPScope;

////////////////////////////////////////////////////////////////////
//       Class : InterrogateType
// Description : An internal representation of a type.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG InterrogateType : public InterrogateComponent {
public:
  InterrogateType(InterrogateModuleDef *def = NULL);
  InterrogateType(const InterrogateType &copy);
  void operator = (const InterrogateType &copy);

  INLINE bool is_global() const;

  INLINE bool has_scoped_name() const;
  INLINE const string &get_scoped_name() const;

  INLINE bool has_true_name() const;
  INLINE const string &get_true_name() const;

  INLINE bool has_comment() const;
  INLINE const string &get_comment() const;

  INLINE bool is_nested() const;
  INLINE TypeIndex get_outer_class() const;

  INLINE bool is_atomic() const;
  INLINE AtomicToken get_atomic_token() const;
  INLINE bool is_unsigned() const;
  INLINE bool is_signed() const;
  INLINE bool is_long() const;
  INLINE bool is_longlong() const;
  INLINE bool is_short() const;

  INLINE bool is_wrapped() const;
  INLINE bool is_pointer() const;
  INLINE bool is_const() const;
  INLINE TypeIndex get_wrapped_type() const;

  INLINE bool is_enum() const;
  INLINE int number_of_enum_values() const;
  INLINE const string &get_enum_value_name(int n) const;
  INLINE const string &get_enum_value_scoped_name(int n) const;
  INLINE int get_enum_value(int n) const;

  INLINE bool is_struct() const;
  INLINE bool is_class() const;
  INLINE bool is_union() const;

  INLINE bool is_fully_defined() const;
  INLINE bool is_unpublished() const;
  INLINE int number_of_constructors() const;
  INLINE FunctionIndex get_constructor(int n) const;
  INLINE bool has_destructor() const;
  INLINE bool destructor_is_inherited() const;
  INLINE FunctionIndex get_destructor() const;
  INLINE int number_of_elements() const;
  INLINE ElementIndex get_element(int n) const;
  INLINE int number_of_methods() const;
  INLINE FunctionIndex get_method(int n) const;
  INLINE int number_of_make_seqs() const;
  INLINE MakeSeqIndex get_make_seq(int n) const;

  INLINE int number_of_casts() const;
  INLINE FunctionIndex get_cast(int n) const;

  INLINE int number_of_derivations() const;
  INLINE TypeIndex get_derivation(int n) const;

  INLINE bool derivation_has_upcast(int n) const;
  INLINE FunctionIndex derivation_get_upcast(int n) const;

  INLINE bool derivation_downcast_is_impossible(int n) const;
  INLINE bool derivation_has_downcast(int n) const;
  INLINE FunctionIndex derivation_get_downcast(int n) const;

  INLINE int number_of_nested_types() const;
  INLINE TypeIndex get_nested_type(int n) const;

  void merge_with(const InterrogateType &other);
  void output(ostream &out) const;
  void input(istream &in);

  void remap_indices(const IndexRemapper &remap);

private:
  enum Flags {
    F_global               = 0x000001,
    F_atomic               = 0x000002,
    F_unsigned             = 0x000004,
    F_signed               = 0x000008,
    F_long                 = 0x000010,
    F_longlong             = 0x000020,
    F_short                = 0x000040,
    F_wrapped              = 0x000080,
    F_pointer              = 0x000100,
    F_const                = 0x000200,
    F_struct               = 0x000400,
    F_class                = 0x000800,
    F_union                = 0x001000,
    F_fully_defined        = 0x002000,
    F_true_destructor      = 0x004000,
    F_private_destructor   = 0x008000,
    F_inherited_destructor = 0x010000,
    F_implicit_destructor  = 0x020000,
    F_nested               = 0x040000,
    F_enum                 = 0x080000,
    F_unpublished          = 0x100000,
  };

public:
  int _flags;

  string _scoped_name;
  string _true_name;
  string _comment;
  TypeIndex _outer_class;
  AtomicToken _atomic_token;
  TypeIndex _wrapped_type;

  typedef vector<FunctionIndex> Functions;
  Functions _constructors;
  FunctionIndex _destructor;

  typedef vector<ElementIndex> Elements;
  Elements _elements;
  Functions _methods;
  Functions _casts;

  typedef vector<MakeSeqIndex> MakeSeqs;
  MakeSeqs _make_seqs;

  enum DerivationFlags {
    DF_upcast               = 0x01,
    DF_downcast             = 0x02,
    DF_downcast_impossible  = 0x04
  };

public:
  // This nested class must be declared public just so we can declare
  // the external ostream and istream I/O operator functions, on the
  // SGI compiler.  Arguably a compiler bug, but what can you do.
  class Derivation {
  public:
    void output(ostream &out) const;
    void input(istream &in);

    int _flags;
    TypeIndex _base;
    FunctionIndex _upcast;
    FunctionIndex _downcast;
  };

private:
  typedef vector<Derivation> Derivations;
  Derivations _derivations;

public:
  // This nested class must also be public, for the same reason.
  class EnumValue {
  public:
    void output(ostream &out) const;
    void input(istream &in);

    string _name;
    string _scoped_name;
    int _value;
  };

private:
  typedef vector<EnumValue> EnumValues;
  EnumValues _enum_values;

  typedef vector<TypeIndex> Types;
  Types _nested_types;

  static string _empty_string;

public:
  // The rest of the members in this class aren't part of the public
  // interface to interrogate, but are used internally as the
  // interrogate database is built.  They are valid only during the
  // session of interrogate that generates the database, and will not
  // be filled in when the database is reloaded from disk.
  CPPType *_cpptype;
  CPPScope *_cppscope;

  friend class InterrogateBuilder;
};

INLINE ostream &operator << (ostream &out, const InterrogateType &type);
INLINE istream &operator >> (istream &in, InterrogateType &type);

INLINE ostream &operator << (ostream &out, const InterrogateType::Derivation &d);
INLINE istream &operator >> (istream &in, InterrogateType::Derivation &d);

INLINE ostream &operator << (ostream &out, const InterrogateType::EnumValue &d);
INLINE istream &operator >> (istream &in, InterrogateType::EnumValue &d);

#include "interrogateType.I"

#include <set>
#include <map>
struct Dtool_PyTypedObject;
typedef std::map< int , Dtool_PyTypedObject *>   RunTimeTypeDictionary;
typedef std::set<int >                           RunTimeTypeList;

EXPCL_DTOOLCONFIG  RunTimeTypeDictionary & GetRunTimeDictionary();
EXPCL_DTOOLCONFIG  RunTimeTypeList & GetRunTimeTypeList();


#endif
