/**
 *  \file   types.h
 *  \author Jason Fernandez
 *  \date   10/18/2017
 *
 *  https://github.com/jfern2011/types
 */

#ifndef __TYPES__
#define __TYPES__

#include <vector>

typedef unsigned long long uint64;
typedef long long          int64;
typedef unsigned int       uint32;
typedef int                int32;
typedef unsigned short     uint16;
typedef short              int16;
typedef unsigned char      uint8;
typedef char               int8;

typedef std::vector<uint64> uint64_v;
typedef std::vector<uint32> uint32_v;

#define MAX_INT32  2147483647
#define MIN_INT32 -2147483648
#define MAX_INT64  9223372036854775807
#define MIN_INT64 -9223372036854775808

#endif
