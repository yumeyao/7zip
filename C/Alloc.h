/* Alloc.h -- Memory allocation functions
2009-02-07 : Igor Pavlov : Public domain */

#ifndef __COMMON_ALLOC_H
#define __COMMON_ALLOC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _SZ_ALLOC_DEBUG
void *MyAlloc(size_t size);
void MyFree(void *address);
#else
#define MyAlloc(size) (size == 0 ? 0 : malloc(size))
#define MyFree(address) (address == 0 ? 0 : free(address))
#endif

#ifdef _WIN32

#ifdef _MSC_VER
#define MY_FAST_CALL __fastcall
#endif

void SetLargePageSize();

void * MY_FAST_CALL MidAlloc(size_t size);
void MY_FAST_CALL MidFree(void *address);
void * MY_FAST_CALL BigAlloc(size_t size);
void MY_FAST_CALL BigFree(void *address);

#else

#define MidAlloc(size) MyAlloc(size)
#define MidFree(address) MyFree(address)
#define BigAlloc(size) MyAlloc(size)
#define BigFree(address) MyFree(address)

#endif

#ifdef __cplusplus
}
#endif

#endif
