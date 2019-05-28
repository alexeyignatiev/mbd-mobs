//jpms:bc
/*----------------------------------------------------------------------------*\
 * File:        basic_types.h
 *
 * Description: Basic types used by BOLT.
 *
 * Author:      jpms
 * 
 * Revision:    $Id$.
 *
 *                                     Copyright (c) 2009, Joao Marques-Silva
\*----------------------------------------------------------------------------*/
//jpms:ec

#ifndef _BASIC_TYPES_H
#define _BASIC_TYPES_H 1

#ifdef EXTRAS_ANTON
#include <cstdint>
#endif

/*----------------------------------------------------------------------------*\
 * Values besides 0 and 1
\*----------------------------------------------------------------------------*/

#ifndef EXTRAS_ANTON
#ifdef __LP64__
typedef unsigned long long int ULINT;
typedef long long int LINT;
#define MAXLINT LLONG_MAX
#define MINLINT LLONG_MIN
#define MAXULINT ULLONG_MAX
#else
typedef unsigned long int ULINT;
typedef long int LINT;
#define MAXLINT LONG_MAX
#define MINLINT LONG_MIN
#define MAXULINT ULONG_MAX
#endif
#else // EXTRAS_ANTON
// 32-bit integers
typedef uint32_t ULINT;
typedef int32_t LINT;
#define MAXLINT INT32_MAX
#define MINLINT INT32_MIN
#define MAXULINT UINT32_MAX
#endif

#ifdef GMPDEF
#include <gmpxx.h>
typedef mpz_class XLINT;
#define ToLint(x) x.get_si()
#else
#ifndef EXTRAS_ANTON
typedef LINT XLINT;
#else
typedef int64_t XLINT;
#endif
#define ToLint(x) (LINT)x
#endif

#endif /* _BASIC_TYPES_H */

/*----------------------------------------------------------------------------*/
