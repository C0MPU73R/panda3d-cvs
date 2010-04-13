// Filename: colladaData.cxx
// Created by:  rdb (13Apr10)
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

#include "colladaData.h"
#include "config_collada.h"
#include "config_util.h"
#include "config_express.h"
#include "string_utils.h"
#include "dSearchPath.h"
#include "virtualFileSystem.h"
#include "zStream.h"

TypeHandle ColladaData::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColladaData::resolve_dae_filename
//       Access: Public, Static
//  Description: Looks for the indicated filename, first along the
//               indicated searchpath, and then along the model_path.
//               If found, updates the filename to the full path and
//               returns true; otherwise, returns false.
////////////////////////////////////////////////////////////////////
bool ColladaData::
resolve_dae_filename(Filename &dae_filename, const DSearchPath &searchpath) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  
  if (dae_filename.is_fully_qualified() && vfs->exists(dae_filename)) {
    return true;
  }
  
  vfs->resolve_filename(dae_filename, searchpath, "dae") ||
    vfs->resolve_filename(dae_filename, get_model_path(), "dae");
  
  return vfs->exists(dae_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaData::read
//       Access: Public
//  Description: Opens the indicated filename and reads the dae data
//               contents from it.  Returns true if the file was
//               successfully opened and read, false if there were
//               some errors, in which case the data may be partially
//               read.
//
//               error is the output stream to which to write error
//               messages.
////////////////////////////////////////////////////////////////////
bool ColladaData::
read(Filename filename, string display_name) {
  filename.set_text();
  set_filename(filename);

  if (display_name.empty()) {
    display_name = filename;
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    
  istream *file = vfs->open_read_file(filename, true);
  if (file == (istream *)NULL) {
    collada_cat.error() << "Unable to open " << display_name << "\n";
    return false;
  }
  
  collada_cat.info()
    << "Reading " << display_name << "\n";
  
  bool read_ok = read(*file);
  vfs->close_read_file(file);
  return read_ok;
}


////////////////////////////////////////////////////////////////////
//     Function: ColladaData::read
//       Access: Public
//  Description: Parses the dae syntax contained in the indicated
//               input stream.  Returns true if the stream was a
//               completely valid dae file, false if there were some
//               errors, in which case the data may be partially read.
//
//               Before you call this routine, you should probably
//               call set_filename() to set the name of the dae
//               file we're processing, if at all possible.  If there
//               is no such filename, you may set it to the empty
//               string.
////////////////////////////////////////////////////////////////////
bool ColladaData::
read(istream &in) {
  // First, dispense with any children we had previously.  We will
  // replace them with the new data.
  //clear();

  // PARSE HERE

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaData::write_dae
//       Access: Public
//  Description: The main interface for writing complete dae files.
////////////////////////////////////////////////////////////////////
bool ColladaData::
write_dae(Filename filename) const {
  filename.unlink();
  filename.set_text();

#ifdef HAVE_ZLIB
  bool pz_file = false;
  if (filename.get_extension() == "pz") {
    // The filename ends in .pz, which means to automatically compress
    // the dae file that we write.
    pz_file = true;
    filename.set_binary();
  }
#endif  // HAVE_ZLIB

  pofstream file;
  if (!filename.open_write(file)) {
    collada_cat.error() << "Unable to open " << filename << " for writing.\n";
    return false;
  }

#ifdef HAVE_ZLIB
  if (pz_file) {
    OCompressStream compressor(&file, false);
    return write_dae(compressor);
  }
#endif  // HAVE_ZLIB

  return write_dae(file);
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaData::write_dae
//       Access: Public
//  Description: The main interface for writing complete dae files.
////////////////////////////////////////////////////////////////////
bool ColladaData::
write_dae(ostream &out, int indent_level) const {
  //write(out, 0);
  return true;
}

