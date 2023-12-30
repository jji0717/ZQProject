// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: ZQ_common_conf.h,v 1.9 2004/06/23 06:42:41 wli Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : Define cofigurations and symbos
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/DODServer/Project/DCA/ZQ_common_conf.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 1     06-10-26 16:49 Li.huang
// 
// 6     05-04-14 23:02 Daniel.wang
// 
// 5     4/13/05 6:30p Hui.shao
// 
// 4     4/13/05 5:06p Hui.shao
// 
// 3     05-01-11 15:41 Daniel.wang
// delete ssize_t type because of so many libraries defined this type
// Revision 1.9  2004/06/23 06:42:41  wli
// restore to ver 1.7 precompile problem
//
// Revision 1.8  2004/06/22 09:01:37  wli
// add _WIN32_WINNT define and WIN32_LEAN_AND_MEAN define
//
// Revision 1.7  2004/05/26 09:32:35  mwang
// no message
//
// Revision 1.6  2004/05/09 03:54:49  shao
// no message
//
// Revision 1.5  2004/04/28 06:25:02  shao
// doxy comment format
//
// Revision 1.4  2004/04/27 08:52:12  shao
// no message
//
// Revision 1.3  2004/04/27 02:29:29  shao
// addjusted file header format
//
// Revision 1.2  2004/04/21 04:24:19  shao
// winsock2
//
// ===========================================================================

#ifndef __ZQ_COMMON_CONF_H__
#define __ZQ_COMMON_CONF_H__


#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

// for windows application compiled with Borland
#ifndef WIN32
#  if defined(__BORLANDC__) && defined(_Windows)
#    define WIN32
#  endif
#  if defined(_WIN32)
#    define WIN32
#  endif
#endif

// check multithreading
#if defined(__BORLANDC__) && !defined(__MT__)
#  error Please enable multithreading
#endif
#if defined(_MSC_VER) && !defined(_MT)
#  error Please enable multithreading
#endif

#ifdef _MSC_VER
// Disable level 4 warnings explicitly, for STL in VC++
#pragma warning (disable: 4800 4355 4786 4503 4275 4251 4290)
#endif

// check DLL compiling
//#ifdef _MSC_VER
//# ifndef _DLL
//#  error Please enable DLL linking (Use Runtime Library)
//# endif
//#endif

// #ifndef SHCXX_WIN32
// #define SHCXX_WIN32
// #endif

#ifdef WIN32

/*
#  ifndef ssize_t
#    define ssize_t int
#  endif
*/

#  ifdef WIN32_MFC
#    include "StdAfx.h"
#  elif defined(_MSC_VER)
#    include <winsock2.h>
#    include <windows.h>
#    include <winbase.h>
#  endif // WIN32_MFC

#endif // WIN32

#undef	__DLLRTL
#undef SHCLASS_EXPORT

#if defined(__MINGW32__) && !defined(__MSVCRT__)
#  define	CCXX_NOMSVCRT
#endif

#if defined(__MINGW32__) || defined(__CYGWIN32__)

// CygWin32 definition
// -----------------------
#define	HAVE_OLD_IOSTREAM

#undef __EXPORT
#undef __stdcall
#define __stdcall
#define	__EXPORT
#define CCXX_EXPORT(t) t
#define CCXX_MEMBER(t) t
#define CCXX_MEMBER_EXPORT(t) t
#define CCXX_CLASS_EXPORT
typedef char int8;
typedef short int16;
typedef long int32;
typedef long long int64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned long long uint64;
#ifdef __MINGW32__
# define HAVE_MODULES   1
# define alloca(x)      __builtin_alloca(x)
# define THROW(x)       throw x
# define THROWS(x)      throw(x)
  typedef unsigned int  uint;
# define        snprintf            _snprintf
# ifndef ETC_PREFIX
#   define ETC_PREFIX   "c:/"
# endif
#else
typedef DWORD size_t;
#endif

#else // !defined(__MINGW32__) && !defined(__CYGWIN32__)

// non-CygWin32 definition
// -----------------------
#define	__DLLRTL  __declspec(dllexport)
#define	__EXPORT  __declspec(dllimport)

#define	snprintf	_snprintf
#define	vsnprintf	_vsnprintf

//#define ZQ_COMMON_API __EXPORT

typedef __int8  int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned int uint;
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

#define HAVE_MODULES 1
#undef  HAVE_PTHREAD_RWLOCK
#undef  PTHREAD_MUTEXTYPE_RECURSIVE

// define endian macros
#define __BYTE_ORDER __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321

#ifdef ZQCOMMON_EXPORTS
#  define ZQ_COMMON_API __declspec(dllexport)
#else
#  define ZQ_COMMON_API __declspec(dllimport)
#endif

#ifndef ZQCOMMON_DLL
#  undef  ZQ_COMMON_API
#  define ZQ_COMMON_API // trust as static build
#endif // ZQCOMMON_DLL

//#error asdf
#ifdef WIN32
	typedef DWORD timeout_t; // msec
	typedef uint64	timeout64_t;	// msec, 64 bits
#else
	typedef	unsigned long	timeout_t;
#endif

#define TIMEOUT_INF ~((timeout_t) 0)

#pragma warning (disable:4786)

#endif

#define	COMMON_TPPORT_TYPE_DEFINED

#ifdef _DEBUG
#  define VODLIBEXT "_d.lib"
#else
#  define VODLIBEXT ".lib"
#endif

// this has already be defined in SeaChange SDK
//#define  __TODO__  __FILE__ " //TODO: "

#  define LOGIC_FNSEPS "/"
#  define LOGIC_FNSEPC '/'

#ifdef WIN32
#  define FNSEPS "\\"
#  define PHSEPS ";"
#  define FNSEPC '\\'
#  define PHSEPC ';'
#else
#  define FNSEPS "/"
#  define PHSEPS ":"
#  define FNSEPC '/'
#  define PHSEPC ':'
#endif

#ifndef  __MSGLOC__
// definitions for macro #pragma message(__MSGLOC__  "blah blah")
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __MSGLOC__ __FILE__ "("__STR1__(__LINE__)") : "
#endif // __MSGLOC__

#ifndef IN
#  define IN
#endif
#ifndef OUT
#  define OUT
#endif

/// The root namespace of ZQ classes
namespace ZQ   {
/// ZQ common c++ classes
namespace common  {



}; // namespace VOD
}; // namespace ZQ

#endif // __ZQ_COMMON_CONF_H__

