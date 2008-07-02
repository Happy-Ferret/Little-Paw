//
// Config.Win32.pp
//
// This file defines some custom config variables for the Windows
// platform, using MS VC++.  It makes some initial guesses about
// compiler features, etc.
//

// *******************************************************************
// NOTE: you should not attempt to copy this file verbatim as your own
// personal Config.pp file.  Instead, you should start with an empty
// Config.pp file, and add lines to it when you wish to override
// settings given in here.  In the normal ppremake system, this file
// will always be read first, and then your personal Config.pp file
// will be read later, which gives you a chance to override the
// default settings found in this file.  However, if you start by
// copying the entire file, it will be difficult to tell which
// settings you have customized, and it will be difficult to upgrade
// to a subsequent version of Panda.
// *******************************************************************

// What additional flags should we pass to interrogate?
#define SYSTEM_IGATE_FLAGS -longlong __int64 -D_X86_ -DWIN32_VC -D"_declspec(param)=" -D"__declspec(param)=" -D_near  -D_far -D__near  -D__far -D_WIN32 -D__stdcall -Dvolatile -Dmutable

// Additional flags to pass to the Tau instrumentor.
#define TAU_INSTRUMENTOR_FLAGS -DTAU_USE_C_API -DPROFILING_ON -DWIN32_VC -D_WIN32 -D__cdecl= -D__stdcall= -D__fastcall= -D__i386 -D_MSC_VER=1310 -D_W64=  -D_INTEGRAL_MAX_BITS=64 --exceptions --late_tiebreaker --no_class_name_injection --no_warnings --restrict --microsoft --new_for_init

// Is the platform big-endian (like an SGI workstation) or
// little-endian (like a PC)?  Define this to the empty string to
// indicate little-endian, or nonempty to indicate big-endian.
#define WORDS_BIGENDIAN

// Does the C++ compiler support namespaces?
#define HAVE_NAMESPACE 1

// Does the C++ compiler support ios::binary?
#define HAVE_IOS_BINARY 1

// How about the typename keyword?
#define HAVE_TYPENAME 1

// Will the compiler avoid inserting extra bytes in structs between a
// base struct and its derived structs?  It is safe to define this
// false if you don't know, but if you know that you can get away with
// this you may gain a tiny performance gain by defining this true.
// If you define this true incorrectly, you will get lots of
// assertion failures on execution.
#define SIMPLE_STRUCT_POINTERS 1

// Do we have a gettimeofday() function?
#define HAVE_GETTIMEOFDAY

// Does gettimeofday() take only one parameter?
#define GETTIMEOFDAY_ONE_PARAM

// Do we have getopt() and/or getopt_long_only() built into the
// system?
#define HAVE_GETOPT
#define HAVE_GETOPT_LONG_ONLY

// Are the above getopt() functions defined in getopt.h, or somewhere else?
#define HAVE_GETOPT_H

// Can we determine the terminal width by making an ioctl(TIOCGWINSZ) call?
#define IOCTL_TERMINAL_WIDTH

// Do the system headers define a "streamsize" typedef?  How about the
// ios::binary enumerated value?  And other ios typedef symbols like
// ios::openmode and ios::fmtflags?
#define HAVE_STREAMSIZE 1
#define HAVE_IOS_BINARY 1
#define HAVE_IOS_TYPEDEFS 1

// Can we safely call getenv() at static init time?
#define STATIC_INIT_GETENV 1

// Can we read the file /proc/self/environ to determine our
// environment variables at static init time?
#define HAVE_PROC_SELF_ENVIRON

// Do we have a global pair of argc/argv variables that we can read at
// static init time?  Should we prototype them?  What are they called?
#define HAVE_GLOBAL_ARGV 1
#define PROTOTYPE_GLOBAL_ARGV
#define GLOBAL_ARGV __argv
#define GLOBAL_ARGC __argc

// Can we read the file /proc/self/cmdline to determine our
// command-line arguments at static init time?
#define HAVE_PROC_SELF_CMDLINE

// Should we include <iostream> or <iostream.h>?  Define HAVE_IOSTREAM
// to nonempty if we should use <iostream>, or empty if we should use
// <iostream.h>.
#define HAVE_IOSTREAM 1

// Do we have a true stringstream class defined in <sstream>?
#define HAVE_SSTREAM 1

// Does fstream::open() require a third parameter, specifying the
// umask?
#define HAVE_OPEN_MASK

// Do the compiler or system libraries define wchar_t for you?
#define HAVE_WCHAR_T 1

// Does <string> define the typedef wstring?  Most do, but for some
// reason, versions of gcc before 3.0 didn't do this.
#define HAVE_WSTRING 1

// Do we have <new>?
#define HAVE_NEW 1

// Do we have <io.h>?
#define HAVE_IO_H 1

// Do we have <malloc.h>?
#define HAVE_MALLOC_H 1

// Do we have <alloca.h>?
#define HAVE_ALLOCA_H

// Do we have <locale.h>?
#define HAVE_LOCALE_H

// Do we have <string.h>?
#define HAVE_STRING_H 1

// Do we have <stdlib.h>?
#define HAVE_STDLIB_H

// Do we have <limits.h>?
#define HAVE_LIMITS_H

// Do we have <minmax.h>?
#define HAVE_MINMAX_H 1

// Do we have <sys/types.h>?
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_TIME_H

// Do we have <unistd.h>?
#define HAVE_UNISTD_H

// Do we have <utime.h>?
#define HAVE_UTIME_H

// Do we have <dirent.h>?
#define HAVE_DIRENT_H

// Do we have <sys/soundcard.h> (and presumably a Linux-style audio
// interface)?
#define HAVE_SYS_SOUNDCARD_H

// Do we have <ucontext.h> (and therefore makecontext() / swapcontext())?
#define HAVE_UCONTEXT_H

// Do we have RTTI (and <typeinfo>)?
#define HAVE_RTTI 1

// MSVC7 does support the latest STL allocator definitions.
#define USE_STL_ALLOCATOR 1

// can Intel C++ build this directory successfully (if not, change CC to msvc)
#define NOT_INTEL_BUILDABLE false

// The dynamic library file extension (usually .so .dll or .dylib):
#define DYNAMIC_LIB_EXT .dll
#define BUNDLE_EXT
  

