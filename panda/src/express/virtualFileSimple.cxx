// Filename: virtualFileSimple.cxx
// Created by:  drose (03Aug02)
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

#include "virtualFileSimple.h"
#include "virtualFileMount.h"
#include "virtualFileList.h"

TypeHandle VirtualFileSimple::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::get_file_system
//       Access: Published, Virtual
//  Description: Returns the VirtualFileSystem this file is associated
//               with.
////////////////////////////////////////////////////////////////////
VirtualFileSystem *VirtualFileSimple::
get_file_system() const {
  return _mount->get_file_system();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::get_filename
//       Access: Published, Virtual
//  Description: Returns the full pathname to this file within the
//               virtual file system.
////////////////////////////////////////////////////////////////////
Filename VirtualFileSimple::
get_filename() const {
  string mount_point = _mount->get_mount_point();
  if (_local_filename.empty()) {
    if (mount_point.empty()) {
      return "/";
    } else {
      return string("/") + mount_point;
    }

  } else {
    if (mount_point.empty()) {
      return string("/") + _local_filename.get_fullpath();
    } else {
      return string("/") + mount_point + string("/") + _local_filename.get_fullpath();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::has_file
//       Access: Published, Virtual
//  Description: Returns true if this file exists, false otherwise.
////////////////////////////////////////////////////////////////////
bool VirtualFileSimple::
has_file() const {
  return _mount->has_file(_local_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::is_directory
//       Access: Published, Virtual
//  Description: Returns true if this file represents a directory (and
//               scan_directory() may be called), false otherwise.
////////////////////////////////////////////////////////////////////
bool VirtualFileSimple::
is_directory() const {
  return _mount->is_directory(_local_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::is_regular_file
//       Access: Published, Virtual
//  Description: Returns true if this file represents a regular file
//               (and read_file() may be called), false otherwise.
////////////////////////////////////////////////////////////////////
bool VirtualFileSimple::
is_regular_file() const {
  return _mount->is_regular_file(_local_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::is_writable
//       Access: Published, Virtual
//  Description: Returns true if this file represents a writable
//               regular file (and write_file() may be called), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool VirtualFileSimple::
is_writable() const {
  return _mount->is_writable(_local_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::open_read_file
//       Access: Published, Virtual
//  Description: Opens the file for reading.  Returns a newly
//               allocated istream on success (which you should
//               eventually delete when you are done reading).
//               Returns NULL on failure.
//
//               If auto_unwrap is true, an explicitly-named .pz file
//               is automatically decompressed and the decompressed
//               contents are returned.  This is different than
//               vfs-implicit-pz, which will automatically decompress
//               a file if the extension .pz is *not* given.
////////////////////////////////////////////////////////////////////
istream *VirtualFileSimple::
open_read_file(bool auto_unwrap) const {

  // Will we be automatically unwrapping a .pz file?
  bool do_uncompress = (_implicit_pz_file || (auto_unwrap && _local_filename.get_extension() == "pz"));

  Filename local_filename(_local_filename);
  if (do_uncompress) {
    // .pz files are always binary, of course.
    local_filename.set_binary();
  }

  return _mount->open_read_file(local_filename, do_uncompress);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::close_read_file
//       Access: Published
//  Description: Closes a file opened by a previous call to
//               open_read_file().  This really just deletes the
//               istream pointer, but it is recommended to use this
//               interface instead of deleting it explicitly, to help
//               work around compiler issues.
////////////////////////////////////////////////////////////////////
void VirtualFileSimple::
close_read_file(istream *stream) const {
  _mount->close_read_file(stream);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::open_write_file
//       Access: Published, Virtual
//  Description: Opens the file for writing.  Returns a newly
//               allocated ostream on success (which you should
//               eventually delete when you are done writing).
//               Returns NULL on failure.
//
//               If auto_wrap is true, an explicitly-named .pz file
//               is automatically compressed.
////////////////////////////////////////////////////////////////////
ostream *VirtualFileSimple::
open_write_file(bool auto_wrap) const {
  // Will we be automatically wrapping a .pz file?
  bool do_compress = (_implicit_pz_file || (auto_wrap && _local_filename.get_extension() == "pz"));

  Filename local_filename(_local_filename);
  if (do_compress) {
    // .pz files are always binary, of course.
    local_filename.set_binary();
  }

  return _mount->open_write_file(local_filename, do_compress);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::close_write_file
//       Access: Published
//  Description: Closes a file opened by a previous call to
//               open_write_file().  This really just deletes the
//               ostream pointer, but it is recommended to use this
//               interface instead of deleting it explicitly, to help
//               work around compiler issues.
////////////////////////////////////////////////////////////////////
void VirtualFileSimple::
close_write_file(ostream *stream) const {
  _mount->close_write_file(stream);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::get_file_size
//       Access: Published, Virtual
//  Description: Returns the current size on disk (or wherever it is)
//               of the already-open file.  Pass in the stream that
//               was returned by open_read_file(); some
//               implementations may require this stream to determine
//               the size.
////////////////////////////////////////////////////////////////////
off_t VirtualFileSimple::
get_file_size(istream *stream) const {
  return _mount->get_file_size(_local_filename, stream);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::get_file_size
//       Access: Published, Virtual
//  Description: Returns the current size on disk (or wherever it is)
//               of the file before it has been opened.
////////////////////////////////////////////////////////////////////
off_t VirtualFileSimple::
get_file_size() const {
  return _mount->get_file_size(_local_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::get_timestamp
//       Access: Published, Virtual
//  Description: Returns a time_t value that represents the time the
//               file was last modified, to within whatever precision
//               the operating system records this information (on a
//               Windows95 system, for instance, this may only be
//               accurate to within 2 seconds).
//
//               If the timestamp cannot be determined, either because
//               it is not supported by the operating system or
//               because there is some error (such as file not found),
//               returns 0.
////////////////////////////////////////////////////////////////////
time_t VirtualFileSimple::
get_timestamp() const {
  return _mount->get_timestamp(_local_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::get_system_info
//       Access: Published, Virtual
//  Description: Populates the SubfileInfo structure with the data
//               representing where the file actually resides on disk,
//               if this is knowable.  Returns true if the file might
//               reside on disk, and the info is populated, or false
//               if it does not (or it is not known where the file
//               resides), in which case the info is meaningless.
////////////////////////////////////////////////////////////////////
bool VirtualFileSimple::
get_system_info(SubfileInfo &info) {
  return _mount->get_system_info(_local_filename, info);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::read_file
//       Access: Public, Virtual
//  Description: Fills up the indicated pvector with the contents of
//               the file, if it is a regular file.  Returns true on
//               success, false otherwise.
////////////////////////////////////////////////////////////////////
bool VirtualFileSimple::
read_file(pvector<unsigned char> &result, bool auto_unwrap) const {

  // Will we be automatically unwrapping a .pz file?
  bool do_uncompress = (_implicit_pz_file || (auto_unwrap && _local_filename.get_extension() == "pz"));

  Filename local_filename(_local_filename);
  if (do_uncompress) {
    // .pz files are always binary, of course.
    local_filename.set_binary();
  }

  return _mount->read_file(local_filename, do_uncompress, result);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::write_file
//       Access: Public, Virtual
//  Description: Writes the indicated data to the file, if it is
//               writable.  Returns true on success, false otherwise.
////////////////////////////////////////////////////////////////////
bool VirtualFileSimple::
write_file(const unsigned char *data, size_t data_size, bool auto_wrap) {
  // Will we be automatically wrapping a .pz file?
  bool do_compress = (_implicit_pz_file || (auto_wrap && _local_filename.get_extension() == "pz"));

  Filename local_filename(_local_filename);
  if (do_compress) {
    // .pz files are always binary, of course.
    local_filename.set_binary();
  }

  return _mount->write_file(local_filename, do_compress, data, data_size);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::scan_local_directory
//       Access: Protected, Virtual
//  Description: Fills file_list up with the list of files that are
//               within this directory, excluding those whose
//               basenames are listed in mount_points.  Returns true
//               if successful, false if the file is not a directory
//               or the directory cannot be read.
////////////////////////////////////////////////////////////////////
bool VirtualFileSimple::
scan_local_directory(VirtualFileList *file_list, 
                     const ov_set<string> &mount_points) const {
  vector_string names;
  if (!_mount->scan_directory(names, _local_filename)) {
    return false;
  }

  // Now the scan above gave us a list of basenames.  Turn these back
  // into VirtualFile pointers.

  // Each of the files returned by the mount will be just a simple
  // file within the same mount tree, unless it is shadowed by a
  // mount point listed in mount_points.

  vector_string::const_iterator ni;
  for (ni = names.begin(); ni != names.end(); ++ni) {
    const string &basename = (*ni);
    if (mount_points.find(basename) == mount_points.end()) {
      Filename filename(_local_filename, basename);
      VirtualFileSimple *file = new VirtualFileSimple(_mount, filename, false, 0);
      file_list->add_file(file);
    }
  }
  
  return true;
}
