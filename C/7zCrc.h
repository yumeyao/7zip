/* 7zCrc.h -- CRC32 calculation
2009-11-21 : Igor Pavlov : Public domain */

#ifndef __7Z_CRC_H
#define __7Z_CRC_H

#include "Types.h"
#include "CpuArch.h"

EXTERN_C_BEGIN

extern UInt32 g_CrcTable[];

/* Call CrcGenerateTable one time before other CRC functions */
void MY_FAST_CALL CrcGenerateTable(void);

#define CRC_INIT_VAL 0xFFFFFFFF
#define CRC_GET_DIGEST(crc) ((crc) ^ CRC_INIT_VAL)
#define CRC_UPDATE_BYTE(crc, b) (g_CrcTable[((crc) ^ (b)) & 0xFF] ^ ((crc) >> 8))

#ifdef MY_CPU_AMD64
UInt32 MY_FAST_CALL CrcUpdateT4(UInt32 v, const void *data, size_t size);
#define g_CrcUpdate CrcUpdateT4
#elif defined(MY_CPU_BE)
UInt32 MY_FAST_CALL CrcUpdateT1_BeT4(UInt32 v, const void *data, size_t size);
#define g_CrcUpdate CrcUpdateT1_BeT4
#else
typedef UInt32 (MY_FAST_CALL *CRC_FUNC)(UInt32 v, const void *data, size_t size);
extern CRC_FUNC g_CrcUpdate;
#endif

MY_INLINE UInt32 CrcUpdate(UInt32 v, const void *data, size_t size)
{
  return g_CrcUpdate(v, data, size);
}

MY_INLINE UInt32 CrcCalc(const void *data, size_t size)
{
  return g_CrcUpdate(CRC_INIT_VAL, data, size) ^ CRC_INIT_VAL;
}

EXTERN_C_END

#endif
