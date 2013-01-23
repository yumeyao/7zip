/* CpuArch.c -- CPU specific code
2010-10-26: Igor Pavlov : Public domain */

#include "CpuArch.h"

#ifdef MY_CPU_X86_OR_AMD64

#if (defined(_MSC_VER) && !defined(MY_CPU_AMD64)) || defined(__GNUC__)
#define USE_ASM
#endif

#ifndef _MSC_VER
#if defined(USE_ASM) && !defined(MY_CPU_AMD64)
static UInt32 CheckFlag(UInt32 flag)
{
  __asm__ __volatile__ (
    "pushf\n\t"
    "pop  %%EAX\n\t"
    "movl %%EAX,%%EDX\n\t"
    "xorl %0,%%EAX\n\t"
    "push %%EAX\n\t"
    "popf\n\t"
    "pushf\n\t"
    "pop  %%EAX\n\t"
    "xorl %%EDX,%%EAX\n\t"
    "push %%EDX\n\t"
    "popf\n\t"
    "andl %%EAX, %0\n\t":
    "=c" (flag) : "c" (flag));
  return flag;
}
#define CHECK_CPUID_IS_SUPPORTED if (CheckFlag(1 << 18 | 1 << 21) != (1 << 18 | 1 << 21)) return False;
#else
#define CHECK_CPUID_IS_SUPPORTED
#endif

static void MyCPUID(UInt32 function, UInt32 *a, UInt32 *b, UInt32 *c, UInt32 *d)
{
  #ifdef USE_ASM
  __asm__ __volatile__ (
    "cpuid"
    : "=a" (*a) ,
      "=b" (*b) ,
      "=c" (*c) ,
      "=d" (*d)
    : "0" (function)) ;
  #else
  int CPUInfo[4];
  __cpuid(CPUInfo, function);
  *a = CPUInfo[0];
  *b = CPUInfo[1];
  *c = CPUInfo[2];
  *d = CPUInfo[3];
  #endif
}

Bool x86cpuid_CheckAndRead(Cx86cpuid *p)
{
  CHECK_CPUID_IS_SUPPORTED
  MyCPUID(0, &p->maxFunc, &p->vendor[0], &p->vendor[2], &p->vendor[1]);
  MyCPUID(1, &p->ver, &p->b, &p->c, &p->d);
  return True;
}
#else
__declspec(naked)
Bool x86cpuid_CheckAndRead(Cx86cpuid *p)
{
  __asm {
  push  esi
  mov   esi, ecx
  pushfd
  mov   ecx, (1 << 18) | (1 << 21)
  pop   eax
  mov   edx, eax
  xor   eax, ecx
  push  eax
  popfd
  pushfd
  pop   eax
  xor   eax, edx
  push  edx
  and   eax, ecx
  popfd
  xor   edx, edx
  sub   ecx, eax
  mov   eax, edx
  jnz   Exit
  push  ebx
  xor   ebx, ebx
  cpuid
  mov   [esi], eax
  mov   [esi+4], ebx
  mov   [esi+8], edx
  mov   [esi+12], ecx
  xor   ecx, ecx
  xor   ebx, ebx
  lea   eax, [ecx+1]
  xor   edx, edx
  cpuid
  mov   [esi+16], eax
  mov   [esi+20], ebx
  pop   ebx
  mov   [esi+24], ecx
  xor   eax, eax
  mov   [esi+28], edx
  inc   eax
Exit:
  pop   esi
  ret
  }
}
#endif

static UInt32 kVendors[][3] =
{
  { 0x756E6547, 0x49656E69, 0x6C65746E},
  { 0x68747541, 0x69746E65, 0x444D4163},
  { 0x746E6543, 0x48727561, 0x736C7561}
};

int x86cpuid_GetFirm(const Cx86cpuid *p)
{
  unsigned i;
  for (i = 0; i < sizeof(kVendors) / sizeof(kVendors[i]); i++)
  {
    const UInt32 *v = kVendors[i];
    if (v[0] == p->vendor[0] &&
        v[1] == p->vendor[1] &&
        v[2] == p->vendor[2])
      return (int)i;
  }
  return -1;
}

Bool CPU_Is_InOrder()
{
  Cx86cpuid p;
  int firm;
  UInt32 family, model;
  if (!x86cpuid_CheckAndRead(&p))
    return True;
  family = x86cpuid_GetFamily(&p);
  model = x86cpuid_GetModel(&p);
  firm = x86cpuid_GetFirm(&p);
  switch (firm)
  {
    case CPU_FIRM_INTEL: return (family < 6 || (family == 6 && model == 0x100C));
    case CPU_FIRM_AMD: return (family < 5 || (family == 5 && (model < 6 || model == 0xA)));
    case CPU_FIRM_VIA: return (family < 6 || (family == 6 && model < 0xF));
  }
  return True;
}

#if !defined(MY_CPU_AMD64) && defined(_WIN32)
static Bool CPU_Sys_Is_SSE_Supported()
{
  OSVERSIONINFO vi;
  vi.dwOSVersionInfoSize = sizeof(vi);
  if (!GetVersionEx(&vi))
    return False;
  return (vi.dwMajorVersion >= 5);
}
#define CHECK_SYS_SSE_SUPPORT if (!CPU_Sys_Is_SSE_Supported()) return False;
#else
#define CHECK_SYS_SSE_SUPPORT
#endif

Bool CPU_Is_Aes_Supported()
{
  Cx86cpuid p;
  CHECK_SYS_SSE_SUPPORT
  if (!x86cpuid_CheckAndRead(&p))
    return False;
  return (p.c >> 25) & 1;
}

#endif
