/*
 * H.265 video codec.
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of libde265.
 *
 * libde265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * libde265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libde265.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DE265_UTIL_H
#define DE265_UTIL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef _MSC_VER
#include <inttypes.h>
#endif

#include <stdio.h>
#include <string>
#include <sstream>
#include <deque>
#include <vector>
#include <assert.h>

#include "libde265/de265.h"

#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#define IS_LITTLE_ENDIAN 1
#else
#include <endian.h>
#define IS_LITTLE_ENDIAN (BYTE_ORDER == LITTLE_ENDIAN)
#endif

#ifdef _MSC_VER
#define LIBDE265_DECLARE_ALIGNED( var, n ) __declspec(align(n)) var
#define likely(x)      (x)
#define unlikely(x)    (x)
#else
#define LIBDE265_DECLARE_ALIGNED( var, n ) var __attribute__((aligned(n)))
#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400
#define LIBDE265_RESTRICT __restrict
#elif !defined(_MSC_VER)
#define LIBDE265_RESTRICT __restrict__
#else
#define LIBDE265_RESTRICT
#endif
#define LIBDE265_RESTRICT_PTR(name) * LIBDE265_RESTRICT name

#if defined(__GNUC__) && (__GNUC__ >= 4)
#define LIBDE265_CHECK_RESULT __attribute__ ((warn_unused_result))
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
#define LIBDE265_CHECK_RESULT _Check_return_
#else
#define LIBDE265_CHECK_RESULT
#endif

#define ALIGNED_32( var ) LIBDE265_DECLARE_ALIGNED( var, 32 )
#define ALIGNED_16( var ) LIBDE265_DECLARE_ALIGNED( var, 16 )
#define ALIGNED_8( var )  LIBDE265_DECLARE_ALIGNED( var, 8 )
#define ALIGNED_4( var )  LIBDE265_DECLARE_ALIGNED( var, 4 )

// C++11 specific features
#if defined(_MSC_VER) || (!__clang__ && __GNUC__ && GCC_VERSION < 40600)
#define FOR_LOOP(type, var, list)   for each (type var in list)
#undef FOR_LOOP_AUTO_SUPPORT
#else
#define FOR_LOOP(type, var, list)   for (type var : list)
#define FOR_LOOP_AUTO_SUPPORT 1
#endif

#ifdef USE_STD_TR1_NAMESPACE
#include <tr1/memory>
namespace std {
  using namespace std::tr1;

  // TODO: put a separate IFDEF around this one

  template <class T, typename... Args> shared_ptr<T> make_shared(Args&& ... args) {
    return shared_ptr<T>( new T(args...) );
  }
}
#endif

#ifdef NEED_STD_MOVE_FALLBACK
// Provide fallback variant of "std::move" for older compilers with
// incomplete/broken C++11 support.
namespace std {

template<typename _Tp>
inline typename std::remove_reference<_Tp>::type&& move(_Tp&& __t) {
  return static_cast<typename std::remove_reference<_Tp>::type&&>(__t);
}

}  // namespace std
#endif

#ifdef NEED_NULLPTR_FALLBACK
// Compilers with partial/incomplete support for C++11 don't know about
// "nullptr". A simple alias should be fine for our use case.
#define nullptr NULL
#endif

#ifdef _MSC_VER
  #ifdef _CPPRTTI
  #define RTTI_ENABLED
  #endif
#else
  #ifdef __GXX_RTTI
  #define RTTI_ENABLED
  #endif
#endif


class image;

//inline uint8_t Clip1_8bit(int16_t value) { if (value<=0) return 0; else if (value>=255) return 255; else return value; }
#define Clip1_8bit(value) ((value)<0 ? 0 : (value)>255 ? 255 : (value))
#define Clip_BitDepth(value, bit_depth) ((value)<0 ? 0 : (value)>((1<<bit_depth)-1) ? ((1<<bit_depth)-1) : (value))
#define Clip3(low,high,value) ((value)<(low) ? (low) : (value)>(high) ? (high) : (value))
#define Sign(value) (((value)<0) ? -1 : ((value)>0) ? 1 : 0)
#define abs_value(a) (((a)<0) ? -(a) : (a))
#define libde265_min(a,b) (((a)<(b)) ? (a) : (b))
#define libde265_max(a,b) (((a)>(b)) ? (a) : (b))

LIBDE265_INLINE static int ceil_div(int num,int denom)
{
  num += denom-1;
  return num/denom;
}

LIBDE265_INLINE static int ceil_log2(int val)
{
  int n=0;
  while (val > (1<<n)) {
    n++;
  }

  return n;
}

LIBDE265_INLINE static int Log2(int v)
{
  int n=0;
  while (v>1) {
    n++;
    v>>=1;
  }

  return n;
}

LIBDE265_INLINE static bool isPowerOf2(int v)
{
  return v>0 && !(v & (v - 1));
}

LIBDE265_INLINE static int Log2SizeToArea(int v)
{
  return (1<<(v<<1));
}

// round 'val' up to the next integer multiple. The factor 'multiple' has to be a power of 2.
LIBDE265_INLINE static int align_up_power_of_two(int val, int multiple)
{
  assert(isPowerOf2(multiple));

  return (val + multiple-1) & ~(multiple-1);
}


void copy_subimage(uint8_t* dst,int dststride,
                   const uint8_t* src,int srcstride,
                   int w, int h);




class error {
 public:
 error(de265_error c) : code(c) { }
 error(de265_error c, const char* m) : code(c), message(m) { }

  de265_error code;

  // optional, additional message (this is not the textual for of 'code', but a more detailed message)
  const char* message = nullptr;
};


class error_queue
{
 public:
  error_queue();

  void add_warning(de265_error warning, bool once=false) {
    add_warning(warning, de265_get_error_text(warning), once);
  }
  void add_warning(de265_error warning, const char* message, bool once=false);

  bool empty() const { return warnings.empty(); }
  error get_next_warning();

 private:
  static constexpr int MAX_WARNING_QUEUE_SIZE = 20;

  std::deque<error> warnings;

  std::vector<error> warnings_shown; // warnings that have already occurred
};





// === logging ===

enum LogModule {
  LogHighlevel,
  LogThreading,
  LogHeaders,
  LogSlice,
  LogDPB,
  LogMotion,
  LogTransform,
  LogDeblock,
  LogSAO,
  LogSEI,
  LogIntraPred,
  LogPixels,
  LogSymbols,
  LogCABAC,
  LogEncoder,
  LogEncoderMetadata,
  NUMBER_OF_LogModules
};


#if defined(DE265_LOG_ERROR) || defined(DE265_LOG_INFO) || defined(DE265_LOG_DEBUG) || defined(DE265_LOG_TRACE)
# define DE265_LOGGING 1
void enable_logging(enum LogModule);
void disable_logging(enum LogModule);
#else
#define enable_logging(x) { }
#define disable_logging(x) { }
#endif

#ifdef DE265_LOGGING
void log_set_current_POC(int poc);
#else
#define log_set_current_POC(poc) { }
#endif

#ifdef DE265_LOG_ERROR
void logerror(enum LogModule module, const char* string, ...);
#else
#define logerror(a,b, ...) do { } while(0)
#endif

#ifdef DE265_LOG_INFO
void loginfo (enum LogModule module, const char* string, ...);
#else
#define loginfo(a,b, ...) do { } while(0)
#endif

#ifdef DE265_LOG_DEBUG
void logdebug(enum LogModule module, const char* string, ...);
bool logdebug_enabled(enum LogModule module);
#else
#define logdebug(a,b, ...) do { } while(0)
inline bool logdebug_enabled(enum LogModule module) { return false; }
#endif

#ifdef DE265_LOG_TRACE
void logtrace(enum LogModule module, const char* string, ...);
#else
#define logtrace(a,b, ...) do { } while(0)
#endif

void log2sstr(std::stringstream& sstr, const char* string, ...);


void printBlk(const char* title,const int32_t* data, int blksize, int stride, const std::string& prefix="  ");
void printBlk(const char* title,const int16_t* data, int blksize, int stride, const std::string& prefix="  ");
void printBlk(const char* title,const uint16_t* data, int blksize, int stride, const std::string& prefix="  ");
void printBlk(const char* title,const uint8_t* data, int blksize, int stride, const std::string& prefix="  ");

void debug_set_image_output(void (*)(const image*, int slot));
void debug_show_image(const class image*, int slot);

double de265_get_time();

#if D_TIMER
class debug_timer
{
 public:
  void start() { mStart = de265_get_time(); }
  void stop()  { mEnd   = de265_get_time(); }
  double get() const { return mEnd - mStart; }
  double get_usecs() const {
    return (mEnd - mStart) * 1.0E6;
  }

 private:
  double mStart,mEnd;
};
#endif

#endif
