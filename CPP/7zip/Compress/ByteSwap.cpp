// ByteSwap.cpp

#include "StdAfx.h"

#include "../../Common/MyCom.h"

#include "../ICoder.h"

#include "../Common/RegisterCodec.h"

class CByteSwap2:
  public ICompressFilter,
  public CMyUnknownImp
{
public:
  MY_UNKNOWN_IMP
  STDMETHOD(Init)();
  STDMETHOD_(UInt32, Filter)(Byte *data, UInt32 size);
};

class CByteSwap4:
  public ICompressFilter,
  public CMyUnknownImp
{
public:
  MY_UNKNOWN_IMP
  STDMETHOD(Init)();
  STDMETHOD_(UInt32, Filter)(Byte *data, UInt32 size);
};

STDMETHODIMP CByteSwap2::Init() { return S_OK; }
STDMETHODIMP CByteSwap4::Init() { return S_OK; }

#if defined(_MSC_VER) && _MSC_VER >= 1400
unsigned short __cdecl _byteswap_ushort(unsigned short);
unsigned long __cdecl _byteswap_ulong(unsigned long);
#pragma intrinsic(_byteswap_ushort)
#pragma intrinsic(_byteswap_ulong)

STDMETHODIMP_(UInt32) CByteSwap2::Filter(Byte *data, UInt32 size)
{
  UInt16 *p_data = (UInt16 *)data;
  UInt16 *p_end = (UInt16 *)(data + size - 2);
  while (p_data <= p_end)
  {
    UInt16 s = *p_data;
	s = _byteswap_ushort(s);
	*p_data = s;
	p_data++;
  }
  return size & ~1;
}

STDMETHODIMP_(UInt32) CByteSwap4::Filter(Byte *data, UInt32 size)
{
  UInt32 *p_data = (UInt32 *)data;
  UInt32 *p_end = (UInt32 *)(data + size - 4);
  while (p_data <= p_end)
  {
    UInt32 l = *p_data;
	l = _byteswap_ulong(l);
	*p_data = l;
	p_data++;
  }
  return size & ~3;
}

#else
STDMETHODIMP_(UInt32) CByteSwap2::Filter(Byte *data, UInt32 size)
{
  const UInt32 kStep = 2;
  UInt32 i;
  for (i = 0; i + kStep <= size; i += kStep)
  {
    Byte b = data[i];
    data[i] = data[i + 1];
    data[i + 1] = b;
  }
  return i;
}

STDMETHODIMP_(UInt32) CByteSwap4::Filter(Byte *data, UInt32 size)
{
  const UInt32 kStep = 4;
  UInt32 i;
  for (i = 0; i + kStep <= size; i += kStep)
  {
    Byte b0 = data[i];
    Byte b1 = data[i + 1];
    data[i] = data[i + 3];
    data[i + 1] = data[i + 2];
    data[i + 2] = b1;
    data[i + 3] = b0;
  }
  return i;
}
#endif

static void *CreateCodec2() { return (void *)(ICompressFilter *)(new CByteSwap2); }
static void *CreateCodec4() { return (void *)(ICompressFilter *)(new CByteSwap4); }

static CCodecInfo g_CodecsInfo[] =
{
  { CreateCodec2, CreateCodec2, 0x020302, L"Swap2", 1, true },
  { CreateCodec4, CreateCodec4, 0x020304, L"Swap4", 1, true }
};

REGISTER_CODECS(ByteSwap)
