#ifndef __TYPES_H
#define __TYPES_H

#include <limits.h>

#if UINTPTR_MAX == UINT32_MAX 
#define PPC32
#define BITS_PER_LONG 32
#elif UINTPTR_MAX == UINT64_MAX
#define PPC64
#define BITS_PER_LONG 64
#else
#error "Unsupported pointer size"
#endif

#define strong_alias(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((alias (#name)));

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

typedef __signed__ long long __s64;
typedef unsigned long long __u64;

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

#define _SIZE_T

#if defined(PPC32)
typedef __u32 size_t;
typedef __s32 ssize_t;
typedef __u32 uintptr_t;
typedef __s32 intptr_t;
#elif defined(PPC64)
typedef __u64 size_t;
typedef __s64 ssize_t;
typedef __u64 uintptr_t;
typedef __s64 intptr_t;
#endif

/* bsd */
typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned int		u_int;
typedef unsigned long		u_long;

/* sysv */
typedef unsigned char		unchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

typedef		__u8		u_int8_t;
typedef		__s8		int8_t;
typedef		__u16		u_int16_t;
typedef		__s16		int16_t;
typedef		__u32		u_int32_t;
typedef		__s32		int32_t;

typedef		__u8		uint8_t;
typedef		__u16		uint16_t;
typedef		__u32		uint32_t;

typedef		__u64		uint64_t;
typedef		__u64		u_int64_t;
typedef		__s64		int64_t;

typedef __u64 ino_t;
typedef __u64 loff_t;
typedef __s64 off_t;
typedef __u64 dev_t;

#endif
