// Common/NewHandler.h

#ifndef __COMMON_NEWHANDLER_H
#define __COMMON_NEWHANDLER_H

class CNewException {};

// #define DEBUG_MEMORY_LEAK

#ifdef _WIN32
#ifndef DEBUG_MEMORY_LEAK
inline
#endif
void
#ifdef _MSC_VER
__cdecl
#endif
operator delete(void *p) throw()
#ifndef DEBUG_MEMORY_LEAK
{
  /*
  if (p == 0)
    return;
  ::HeapFree(::GetProcessHeap(), 0, p);
  */
  ::free(p);
}
#else
;
#endif
#endif

#endif
