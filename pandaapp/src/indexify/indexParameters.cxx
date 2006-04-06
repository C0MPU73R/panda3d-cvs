// Filename: indexParameters.cxx
// Created by:  drose (04Apr02)
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

#include "indexParameters.h"

// User parameters
int max_index_size_array[2] = { 700, 700 };
int thumb_size_array[2] = { 150, 150 };

int thumb_caption_height = 12;
int caption_font_size = 12;

int thumb_space_array[2] = { 12, 12 };

double frame_reduction_factor = 0.75;
int frame_outer_bevel = 2;
int frame_inner_bevel = 1;

int reduced_size_array[2] = { 800, 700 };

Filename archive_dir;

Filename prev_icon;
Filename next_icon;
Filename up_icon;
Filename movie_icon;
Filename sound_icon;

////////////////////////////////////////////////////////////////////
//     Function: get_movie_icon
//  Description: Reads the movie icon filename and returns it as an
//               image, if possible.  Returns a valid image if
//               successful, an invalid one otherwise.
////////////////////////////////////////////////////////////////////
const PNMImage &
get_movie_icon() {
  static PNMImage movie_icon_image;
  static bool got_image = false;

  if (!got_image) {
    if (!movie_icon.empty()) {
      Filename icon_filename(archive_dir, movie_icon);
      nout << "Reading " << icon_filename << "\n";
      movie_icon_image.read(icon_filename);
    }
    got_image = true;
  }

  return movie_icon_image;
}

////////////////////////////////////////////////////////////////////
//     Function: get_sound_icon
//  Description: Reads the sound icon filename and returns it as an
//               image, if possible.  Returns a valid image if
//               successful, an invalid one otherwise.
////////////////////////////////////////////////////////////////////
const PNMImage &
get_sound_icon() {
  static PNMImage sound_icon_image;
  static bool got_image = false;

  if (!got_image) {
    if (!sound_icon.empty()) {
      Filename icon_filename(archive_dir, sound_icon);
      nout << "Reading " << icon_filename << "\n";
      sound_icon_image.read(icon_filename);
    }
    got_image = true;
  }

  return sound_icon_image;
}

bool force_regenerate = false;
bool format_rose = false;
bool sort_date = false;
bool dummy_mode = false;
bool draw_frames = false;
bool omit_roll_headers = false;
DSearchPath cm_search;
bool omit_full_links = false;
bool omit_complete = false;
bool caption_frame_numbers = false;


// Computed parameters
int thumb_count_x;
int thumb_count_y;
int max_thumbs;
int actual_index_width;

int thumb_interior_width;
int thumb_interior_height;

////////////////////////////////////////////////////////////////////
//     Function: finalize_parameters
//  Description: This is called after all user parameters have been
//               changed, to do whatever computations are required for
//               the other parameters that are based on the user
//               parameters.
////////////////////////////////////////////////////////////////////
void
finalize_parameters() {
  thumb_count_x = 
    (max_index_width - thumb_x_space) / (thumb_width + thumb_x_space);
  thumb_count_y = 
    (max_index_height - thumb_y_space) / (thumb_height + thumb_caption_height + thumb_y_space);

  max_thumbs = thumb_count_x * thumb_count_y;
  
  actual_index_width = thumb_x_space + thumb_count_x * (thumb_width + thumb_x_space);

  if (draw_frames) {
    thumb_interior_width = (int)(thumb_width * frame_reduction_factor + 0.5);
    thumb_interior_height = (int)(thumb_height * frame_reduction_factor + 0.5);
  } else {
    thumb_interior_width = thumb_width;
    thumb_interior_height = thumb_height;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: compose_href
//  Description: Combines a user-supplied prefix with a relative
//               directory to generate the appropriate href to the
//               file.
//
//               rel_dir is the relative path to archive_dir.
//               user_prefix is the string the user indicated as the
//               relative or absolute path to the file's parent
//               directory from archive_dir.  basename is the name
//               of the file, or empty if the filename is part of
//               user_prefix.
////////////////////////////////////////////////////////////////////
Filename
compose_href(const Filename &rel_dir, const Filename &user_prefix,
             const Filename &basename) {
  Filename result;

  if (user_prefix.empty()) {
    result = rel_dir;

  } else {
    // Check to see if the user prefix begins with a URL designator,
    // like http:// or ftp://.
    size_t ui = 0;
    while (ui < user_prefix.length() && isalpha(user_prefix[ui])) {
      ui++;
    }
    bool is_url = (user_prefix.get_fullpath().substr(ui, 3) == "://");
    
    if (!is_url && user_prefix.is_local()) {
      Filename rel_user_dir(rel_dir, user_prefix);
      result = rel_user_dir;
      result.standardize();
      
    } else {
      result = user_prefix;
    }
  }

  if (basename.empty()) {
    return result;
  } else {
    return Filename(result, basename);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: escape_html
//  Description: Returns the input string with all invalid characters
//               for HTML code replaced by their HTML equivalents.
////////////////////////////////////////////////////////////////////
string
escape_html(const string &input) {
  static const struct {
    const char *name;
    int code;
  } tokens[] = {
    { "amp", '&' }, { "lt", '<' }, { "gt", '>' }, { "quot", '"' },
    { "nbsp", 160 },

    { "iexcl", 161 }, { "cent", 162 }, { "pound", 163 }, { "curren", 164 },
    { "yen", 165 }, { "brvbar", 166 }, { "brkbar", 166 }, { "sect", 167 },
    { "uml", 168 }, { "die", 168 }, { "copy", 169 }, { "ordf", 170 },
    { "laquo", 171 }, { "not", 172 }, { "shy", 173 }, { "reg", 174 },
    { "macr", 175 }, { "hibar", 175 }, { "deg", 176 }, { "plusmn", 177 },
    { "sup2", 178 }, { "sup3", 179 }, { "acute", 180 }, { "micro", 181 },
    { "para", 182 }, { "middot", 183 }, { "cedil", 184 }, { "sup1", 185 },
    { "ordm", 186 }, { "raquo", 187 }, { "frac14", 188 }, { "frac12", 189 },
    { "frac34", 190 }, { "iquest", 191 }, { "Agrave", 192 }, { "Aacute", 193 },
    { "Acirc", 194 }, { "Atilde", 195 }, { "Auml", 196 }, { "Aring", 197 },
    { "AElig", 198 }, { "Ccedil", 199 }, { "Egrave", 200 }, { "Eacute", 201 },
    { "Ecirc", 202 }, { "Euml", 203 }, { "Igrave", 204 }, { "Iacute", 205 },
    { "Icirc", 206 }, { "Iuml", 207 }, { "ETH", 208 }, { "Dstrok", 208 },
    { "Ntilde", 209 }, { "Ograve", 210 }, { "Oacute", 211 }, { "Ocirc", 212 },
    { "Otilde", 213 }, { "Ouml", 214 }, { "times", 215 }, { "Oslash", 216 },
    { "Ugrave", 217 }, { "Uacute", 218 }, { "Ucirc", 219 }, { "Uuml", 220 },
    { "Yacute", 221 }, { "THORN", 222 }, { "szlig", 223 }, { "agrave", 224 },
    { "aacute", 225 }, { "acirc", 226 }, { "atilde", 227 }, { "auml", 228 },
    { "aring", 229 }, { "aelig", 230 }, { "ccedil", 231 }, { "egrave", 232 },
    { "eacute", 233 }, { "ecirc", 234 }, { "euml", 235 }, { "igrave", 236 },
    { "iacute", 237 }, { "icirc", 238 }, { "iuml", 239 }, { "eth", 240 },
    { "ntilde", 241 }, { "ograve", 242 }, { "oacute", 243 }, { "ocirc", 244 },
    { "otilde", 245 }, { "ouml", 246 }, { "divide", 247 }, { "oslash", 248 },
    { "ugrave", 249 }, { "uacute", 250 }, { "ucirc", 251 }, { "uuml", 252 },
    { "yacute", 253 }, { "thorn", 254 }, { "yuml", 255 },

    { NULL, 0 },
  };

  string result;
  for (string::const_iterator ii = input.begin();
       ii != input.end();
       ++ii) {
    int code = (unsigned char)(*ii);
    bool found_match = false;
    for (int i = 0; !found_match && tokens[i].name != NULL; i++) {
      if (code == tokens[i].code) {
        result += '&';
        result += tokens[i].name;
        result += ';';
        found_match = true;
      }
    }

    if (!found_match) {
      result += (*ii);
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: copy_file
//  Description: Copies a file into the named directory, if it does
//               not already exist there.  Returns true if
//               successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool
copy_file(const Filename &source_file, const Filename &dest_dir) {
  Filename dest_file = source_file;
  dest_file.set_dirname(dest_dir);

  if (dest_file.exists()) {
    return true;
  }

  if (!source_file.exists()) {
    return false;
  }

#if defined(HAVE_UNISTD_H) && !defined(WIN32)
  // Try to make a hard link.
  string os_source_file = source_file.to_os_specific();
  string os_dest_file = dest_file.to_os_specific();
  if (link(os_source_file.c_str(), os_dest_file.c_str()) == 0) {
    cerr << "Linked " << source_file << " to " << dest_file << "\n";
    return true;
  }
#endif

  ifstream in;
  if (!source_file.open_read(in)) {
    cerr << "Unable to read " << source_file << "\n";
    return false;
  }
  ofstream out;
  if (!dest_file.open_write(out)) {
    cerr << "Unable to write " << dest_file << "\n";
    return false;
  }

  cerr << "Copying " << source_file << " to " << dest_file << "\n";

  int c = in.get();
  while (in && !in.eof()) {
    out.put(c);
    c = in.get();
  }

  return in.eof();
}
