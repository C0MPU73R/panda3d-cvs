// Filename: fftCompressor.cxx
// Created by:  drose (11Dec00)
// 
////////////////////////////////////////////////////////////////////

#include "fftCompressor.h"
#include "config_mathutil.h"

#include <datagram.h>
#include <datagramIterator.h>
#include <compose_matrix.h>
#include <math.h>

#ifdef HAVE_FFTW

#include <rfftw.h>

// These FFTW support objects can only be defined if we actually have
// the FFTW library available.
static rfftw_plan get_real_compress_plan(int length);
static rfftw_plan get_real_decompress_plan(int length);

typedef map<int, rfftw_plan> RealPlans;
static RealPlans _real_compress_plans;
static RealPlans _real_decompress_plans;

#endif

////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::Constructor
//       Access: Public
//  Description: Constructs a new compressor object with default
//               parameters.
////////////////////////////////////////////////////////////////////
FFTCompressor::
FFTCompressor() {
  set_quality(-1);
}

////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::is_compression_available
//       Access: Public, Static
//  Description: Returns true if the FFTW library is compiled in, so
//               that this class is actually capable of doing useful
//               compression/decompression work.  Returns false
//               otherwise, in which case any attempt to write a
//               compressed stream will actually write an uncompressed
//               stream, and any attempt to read a compressed stream
//               will fail.
////////////////////////////////////////////////////////////////////
bool FFTCompressor::
is_compression_available() {
#ifndef HAVE_FFTW
  return false;
#else
  return true;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::set_quality
//       Access: Public
//  Description: Sets the quality factor for the compression.  This is
//               an integer in the range 0 - 100 that roughly controls
//               how aggressively the reals are compressed; lower
//               numbers mean smaller output, and more data loss.
//
//               As a special case, a negative quality indicates that
//               the individual parameters should be separately
//               controlled via config variables, and a quality
//               greater than 100 indicates lossless output.
////////////////////////////////////////////////////////////////////
void FFTCompressor::
set_quality(int quality) {
#ifndef HAVE_FFTW
  // If we don't actually have FFTW, we can't really compress anything.
  if (_quality <= 100) {
    mathutil_cat.warning()
      << "FFTW library is not available; generating uncompressed output.\n";
  }
  _quality = 101;

#else
  _quality = quality;

  if (_quality < 0) {
    // A negative quality indicates to read the important parameters
    // from config variables.
    _fft_offset = fft_offset;
    _fft_factor = fft_factor;
    _fft_exponent = fft_exponent;
  } else if (_quality < 40) {
    // 0 - 40 : 
    //   fft-offset 1.0 - 0.001
    //   fft-factor 1.0
    //   fft-exponent 4.0

    double t = (double)_quality / 40.0;
    _fft_offset = interpolate(t, 1.0, 0.001);
    _fft_factor = 1.0;
    _fft_exponent = 4.0;

  } else if (_quality < 95) {
    // 40 - 95:
    //   fft-offset 0.001
    //   fft-factor 1.0 - 0.1
    //   fft-exponent 4.0

    double t = (double)(_quality - 40) / 55.0;
    _fft_offset = 0.001;
    _fft_factor = interpolate(t, 1.0, 0.1);
    _fft_exponent = 4.0;

  } else {
    // 95 - 100:
    //   fft-offset 0.001
    //   fft-factor 0.1 - 0.0
    //   fft-exponent 4.0

    double t = (double)(_quality - 95) / 5.0;
    _fft_offset = 0.001;
    _fft_factor = interpolate(t, 0.1, 0.0);
    _fft_exponent = 4.0;
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::get_quality
//       Access: Public
//  Description: Returns the quality number that was previously set
//               via set_quality().
////////////////////////////////////////////////////////////////////
int FFTCompressor::
get_quality() const {
  return _quality;
}

////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::write_header
//       Access: Public
//  Description: Writes the compression parameters to the indicated
//               datagram.  It is necessary to call this before
//               writing anything else to the datagram, since these
//               parameters will be necessary to correctly decompress
//               the data later.
////////////////////////////////////////////////////////////////////
void FFTCompressor::
write_header(Datagram &datagram) {
  datagram.add_int8(_quality);
  if (_quality < 0) {
    datagram.add_float64(_fft_offset);
    datagram.add_float64(_fft_factor);
    datagram.add_float64(_fft_exponent);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::write_reals
//       Access: Public
//  Description: Writes an array of floating-point numbers to the
//               indicated datagram.
////////////////////////////////////////////////////////////////////
void FFTCompressor::
write_reals(Datagram &datagram, const float *array, int length) {
  datagram.add_int32(length);

  if (_quality > 100) {
    // Special case: lossless output.
    for (int i = 0; i < length; i++) {
      datagram.add_float32(array[i]);
    }
    return;
  }

#ifndef HAVE_FFTW
  // If we don't have FFTW, we shouldn't get here.
  nassertv(false);

#else 

  if (length == 0) {
    // Special case: do nothing.
    return;
  }

  if (length == 1) {
    // Special case: just write out the one number.
    datagram.add_float32(array[0]);
    return;
  }
    
  // Normal case: FFT the array, and write that out.
  double data[length];
  int i;
  for (i = 0; i < length; i++) {
    data[i] = array[i];
  }

  double half_complex[length];

  rfftw_plan plan = get_real_compress_plan(length);
  rfftw_one(plan, data, half_complex);

  if (mathutil_cat.is_debug()) {
    mathutil_cat.debug()
      << "write_reals :";
    for (int i = 0; i < length; i++) {
      double scale_factor = get_scale_factor(i, length);
      mathutil_cat.debug(false) 
	//	<< " " << data[i];
	<< " " << floor(half_complex[i] / scale_factor + 0.5);
    }
    mathutil_cat.debug(false) << "\n";
  }

  // Now encode the numbers, run-length encoded by size, so we only
  // write out the number of bits we need for each number.

  vector_double run;
  RunWidth run_width = RW_invalid;
  int num_written = 0;

  for (i = 0; i < length; i++) {
    static const double max_range_32 = 2147483647.0;
    static const double max_range_16 = 32767.0;
    static const double max_range_8 = 127.0;

    double scale_factor = get_scale_factor(i, length);
    double num = floor(half_complex[i] / scale_factor + 0.5);

    // How many bits do we need to encode this integer?
    double a = fabs(num);
    RunWidth num_width;

    if (a == 0.0) {
      num_width = RW_0;
      
    } else if (a <= max_range_8) {
      num_width = RW_8;

    } else if (a <= max_range_16) {
      num_width = RW_16;

    } else if (a <= max_range_32) {
      num_width = RW_32;

    } else {
      num_width = RW_double;
    }

    // A special case: if we're writing a string of one-byters and we
    // come across a single intervening zero, don't interrupt the run
    // just for that.
    if (run_width == RW_8 && num_width == RW_0) {
      if (i + 1 >= length || half_complex[i + 1] != 0.0) {
	num_width = RW_8;
      }
    }

    if (num_width != run_width) {
      // Now we need to flush the last run.
      num_written += write_run(datagram, run_width, run);
      run.clear();
      run_width = num_width;
    }

    run.push_back(num);
  }

  num_written += write_run(datagram, run_width, run);
  nassertv(num_written == length);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::write_hprs
//       Access: Public
//  Description: Writes an array of HPR angles to the indicated
//               datagram.
////////////////////////////////////////////////////////////////////
void FFTCompressor::
write_hprs(Datagram &datagram, const LVecBase3f *array, int length) {
  // First, convert the HPR's to quats.  We expect quats to have
  // better FFT consistency, and therefore compress better, even
  // though they have an extra component.

  // However, because the quaternion will be normalized, we don't even
  // have to write out all three components; any three can be used to
  // determine the fourth (provided we ensure consistency of sign).

  vector_float qi, qj, qk;

  for (int i = 0; i < length; i++) {
    LMatrix3f mat;
    compose_matrix(mat, LVecBase3f(1.0, 1.0, 1.0), array[i]);
    LOrientationf rot;
    rot.set(mat);
    rot.normalize();

    if (rot.get_r() < 0) {
      // Since rot == -rot, we can flip the quarternion if need be to
      // keep the r component positive.  This has two advantages.
      // One, it makes it possible to infer r completely given i, j,
      // and k (since we know it must be >= 0), and two, it helps
      // protect against poor continuity caused by inadvertent
      // flipping of the quarternion's sign between frames.

      // The choice of leaving r implicit rather than any of the other
      // three seems to work the best in terms of guaranteeing
      // continuity.
      rot.set(-rot.get_r(), -rot.get_i(), -rot.get_j(), -rot.get_k());
    }

    qi.push_back(rot.get_i());
    qj.push_back(rot.get_j());
    qk.push_back(rot.get_k());
  }

  write_reals(datagram, &qi[0], length);
  write_reals(datagram, &qj[0], length);
  write_reals(datagram, &qk[0], length);
}

////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::read_header
//       Access: Public
//  Description: Reads the compression header that was written
//               previously.  This fills in the compression parameters
//               necessary to correctly decompress the following data.
//
//               Returns true if the header is read successfully,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool FFTCompressor::
read_header(DatagramIterator &di) {
  _quality = di.get_int8();

#ifndef HAVE_FFTW
  if (_quality <= 100) {
    mathutil_cat.error()
      << "FFTW library is not available; cannot read compressed data.\n";
    return false;
  }
#endif

  set_quality(_quality);

  if (_quality < 0) {
    _fft_offset = di.get_float64();
    _fft_factor = di.get_float64();
    _fft_exponent = di.get_float64();
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::read_reals
//       Access: Public
//  Description: Reads an array of floating-point numbers.  The result
//               is pushed onto the end of the indicated vector, which
//               is not cleared first; it is the user's responsibility
//               to ensure that the array is initially empty.  Returns
//               true if the data is read correctly, false if there is
//               an error.
////////////////////////////////////////////////////////////////////
bool FFTCompressor::
read_reals(DatagramIterator &di, vector_float &array) {
  int length = di.get_int32();

  if (_quality > 100) {
    // Special case: lossless output.
    for (int i = 0; i < length; i++) {
      array.push_back(di.get_float32());
    }
    return true;
  }

#ifndef HAVE_FFTW
  // If we don't have FFTW, we shouldn't get here.
  return false;

#else

  if (length == 0) {
    // Special case: do nothing.
    return true;
  }

  if (length == 1) {
    // Special case: just read in the one number.
    array.push_back(di.get_float32());
    return true;
  }

  // Normal case: read in the FFT array, and convert it back to
  // (nearly) the original numbers.
  vector_double half_complex;
  half_complex.reserve(length);
  int num_read = 0;
  while (num_read < length) {
    num_read += read_run(di, half_complex);
  }
  nassertr(num_read == length, false);
  nassertr((int)half_complex.size() == length, false);

  int i;
  for (i = 0; i < length; i++) {
    half_complex[i] *= get_scale_factor(i, length);
  }

  double data[length];
  rfftw_plan plan = get_real_decompress_plan(length);
  rfftw_one(plan, &half_complex[0], data);

  double scale = 1.0 / (double)length;
  array.reserve(array.size() + length);
  for (i = 0; i < length; i++) {
    array.push_back(data[i] * scale);
  }

  if (mathutil_cat.is_debug()) {
    mathutil_cat.debug()
      << "read_reals :";
    for (int i = 0; i < length; i++) {
      mathutil_cat.debug(false)
	<< " " << data[i] * scale;
    }
    mathutil_cat.debug(false) << "\n";
  }

  return true;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::read_hprs
//       Access: Public
//  Description: Reads an array of HPR angles.  The result is pushed
//               onto the end of the indicated vector, which is not
//               cleared first; it is the user's responsibility to
//               ensure that the array is initially empty.
////////////////////////////////////////////////////////////////////
bool FFTCompressor::
read_hprs(DatagramIterator &di, vector_LVecBase3f &array) {
  vector_float qi, qj, qk;

  bool okflag = true;

  okflag = 
    read_reals(di, qi) &&
    read_reals(di, qj) &&
    read_reals(di, qk);

  if (okflag) {
    nassertr(qi.size() == qj.size() && qj.size() == qk.size(), false);
    
    array.reserve(array.size() + qi.size());
    for (int i = 0; i < (int)qi.size(); i++) {
      float qr2 = 1.0 - (qi[i] * qi[i] + qj[i] * qj[i] + qk[i] * qk[i]);
      float qr = qr2 < 0.0 ? 0.0 : sqrtf(qr2);

      LOrientationf rot(qr, qi[i], qj[i], qk[i]);
      rot.normalize();      // Just for good measure.

      LMatrix3f mat;
      rot.extract_to_matrix(mat);
      LVecBase3f scale, hpr;
      bool success = decompose_matrix(mat, scale, hpr);
      nassertr(success, false);
      array.push_back(hpr);
    }
  }

  return okflag;
}


////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::free_storage
//       Access: Public, Static
//  Description: Frees memory that has been allocated during past runs
//               of the FFTCompressor.  This is an optional call, but
//               it may be made from time to time to empty the global
//               cache that the compressor objects keep to facilitate
//               fast compression/decompression.
////////////////////////////////////////////////////////////////////
void FFTCompressor::
free_storage() {
  RealPlans::iterator pi;
  for (pi = _real_compress_plans.begin(); 
       pi != _real_compress_plans.end();
       ++pi) {
    rfftw_destroy_plan((*pi).second);
  }
  _real_compress_plans.clear();

  for (pi = _real_decompress_plans.begin(); 
       pi != _real_decompress_plans.end(); 
       ++pi) {
    rfftw_destroy_plan((*pi).second);
  }
  _real_decompress_plans.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::write_run
//       Access: Private
//  Description: Writes a sequence of integers that all require the
//               same number of bits.  Returns the number of integers
//               written, i.e. run.size().
////////////////////////////////////////////////////////////////////
int FFTCompressor::
write_run(Datagram &datagram, FFTCompressor::RunWidth run_width,
	  const vector_double &run) {
  if (run.empty()) {
    return 0;
  }
  nassertr(run_width != RW_invalid, 0);

  if (run_width != RW_double) {
    // If the width is anything other than RW_double, we write a
    // single byte indicating the width and length of the upcoming
    // run.

    if (run.size() <= RW_length_mask &&
	((int)run_width | run.size()) != RW_double) {
      // If there are enough bits remaining in the byte, use them to
      // indicate the length of the run.  We have to be a little
      // careful, however, not to accidentally write a byte that looks
      // like an RW_double flag.
      datagram.add_uint8((int)run_width | run.size());
      
    } else {
      // Otherwise, write zero as the length, to indicate that we'll
      // write the actual length in the following 16-bit word.
      datagram.add_uint8(run_width);
      
      // Assuming, of course, that the length fits within 16 bits.
      nassertr(run.size() < 65536, 0);
      nassertr(run.size() != 0, 0);
      
      datagram.add_uint16(run.size());
    }
  }

  // Now write the data itself.
  vector_double::const_iterator ri;
  switch (run_width) {
  case RW_0:
    // If it's a string of zeroes, we're done!
    break;

  case RW_8:
    for (ri = run.begin(); ri != run.end(); ++ri) {
      datagram.add_int8((int)*ri);
    }
    break;

  case RW_16:
    for (ri = run.begin(); ri != run.end(); ++ri) {
      datagram.add_int16((int)*ri);
    }
    break;

  case RW_32:
    for (ri = run.begin(); ri != run.end(); ++ri) {
      datagram.add_int32((int)*ri);
    }
    break;

  case RW_double:
    for (ri = run.begin(); ri != run.end(); ++ri) {
      // In the case of RW_double, we only write the numbers one at a
      // time, each time preceded by the RW_double flag.  Hopefully
      // this will happen only rarely.
      datagram.add_int8((int)RW_double);
      datagram.add_float64(*ri);
    }
    break;

  default:
    break;
  }

  return run.size();
}

////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::read_run
//       Access: Private
//  Description: Reads a sequence of integers that all require the
//               same number of bits.  Returns the number of integers
//               read.  It is the responsibility of the user to clear
//               the vector before calling this function, or the
//               numbers read will be appended to the end.
////////////////////////////////////////////////////////////////////
int FFTCompressor::
read_run(DatagramIterator &di, vector_double &run) {
  PN_uint8 start = di.get_uint8();
  RunWidth run_width;
  int length;

  if ((start & 0xff) == RW_double) {
    // RW_double is a special case, and requires the whole byte.  In
    // this case, we don't encode a length, but assume it's only one.
    run_width = RW_double;
    length = 1;

  } else {
    run_width = (RunWidth)(start & RW_width_mask);
    length = start & RW_length_mask;
  }

  if (length == 0) {
    // If the length was zero, it means the actual length follows as a
    // 16-bit word.
    length = di.get_uint16();
  }
  nassertr(length != 0, 0);

  run.reserve(run.size() + length);

  int i;
  switch (run_width) {
  case RW_0:
    for (i = 0; i < length; i++) {
      run.push_back(0.0);
    }
    break;

  case RW_8:
    for (i = 0; i < length; i++) {
      run.push_back((double)(int)di.get_int8());
    }
    break;

  case RW_16:
    for (i = 0; i < length; i++) {
      run.push_back((double)(int)di.get_int16());
    }
    break;

  case RW_32:
    for (i = 0; i < length; i++) {
      run.push_back((double)(int)di.get_int32());
    }
    break;

  case RW_double:
    for (i = 0; i < length; i++) {
      run.push_back(di.get_float64());
    }
    break;

  default:
    break;
  }

  return length;
}

////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::get_scale_factor
//       Access: Private
//  Description: Returns the appropriate scaling for the given
//               position within the halfcomplex array.
////////////////////////////////////////////////////////////////////
double FFTCompressor::
get_scale_factor(int i, int length) const {
  int m = (length / 2) + 1;
  int k = (i < m) ? i : length - i;
  nassertr(k >= 0 && k < m, 1.0);

  return _fft_offset + 
    _fft_factor * pow((double)(m-1 - k) / (double)(m-1), _fft_exponent);
}

////////////////////////////////////////////////////////////////////
//     Function: FFTCompressor::interpolate
//       Access: Private, Static
//  Description: Returns a number between a and b, inclusive,
//               according to the value of t between 0 and 1,
//               inclusive.
////////////////////////////////////////////////////////////////////
double FFTCompressor::
interpolate(double t, double a, double b) {
  return a + t * (b - a);
}


#ifdef HAVE_FFTW

////////////////////////////////////////////////////////////////////
//     Function: get_real_compress_plan
//  Description: Returns a FFTW plan suitable for compressing a float
//               array of the indicated length.
////////////////////////////////////////////////////////////////////
static rfftw_plan
get_real_compress_plan(int length) {
  RealPlans::iterator pi;
  pi = _real_compress_plans.find(length);
  if (pi != _real_compress_plans.end()) {
    return (*pi).second;
  }

  rfftw_plan plan;
  plan = rfftw_create_plan(length, FFTW_REAL_TO_COMPLEX, FFTW_MEASURE);
  _real_compress_plans.insert(RealPlans::value_type(length, plan));

  return plan;
}

////////////////////////////////////////////////////////////////////
//     Function: get_real_decompress_plan
//  Description: Returns a FFTW plan suitable for decompressing a float
//               array of the indicated length.
////////////////////////////////////////////////////////////////////
static rfftw_plan
get_real_decompress_plan(int length) {
  RealPlans::iterator pi;
  pi = _real_decompress_plans.find(length);
  if (pi != _real_decompress_plans.end()) {
    return (*pi).second;
  }

  rfftw_plan plan;
  plan = rfftw_create_plan(length, FFTW_COMPLEX_TO_REAL, FFTW_MEASURE);
  _real_decompress_plans.insert(RealPlans::value_type(length, plan));

  return plan;
}

#endif
