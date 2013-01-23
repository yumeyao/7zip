/* 7zCrc.h -- CRC32 calculation
2009-11-21 : Igor Pavlov : Public domain */

#ifndef __7Z_CRC_H
#define __7Z_CRC_H

#include "Types.h"

EXTERN_C_BEGIN

extern UInt32 g_CrcTable[];

/* Call CrcGenerateTable one time before other CRC functions */
void MY_FAST_CALL CrcGenerateTable(void);

#define CRC_INIT_VAL 0xFFFFFFFF
#define CRC_GET_DIGEST(crc) ((crc) ^ CRC_INIT_VAL)
#define CRC_UPDATE_BYTE(crc, b) (g_CrcTable[((crc) ^ (b)) & 0xFF] ^ ((crc) >> 8))

typedef UInt32 (MY_FAST_CALL *CRC_FUNC)(UInt32 v, const void *data, size_t size);
extern CRC_FUNC g_CrcUpdate;

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
