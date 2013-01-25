#define _INC_STRING
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
wchar_t * __cdecl wcscpy(wchar_t *, const wchar_t *);

#include <stdio.h>
#include <windows.h>

int __cdecl memcmp(const void *, const void *, int);
__declspec(dllimport) int __cdecl strlen(const char *);
__declspec(dllimport) int memcpy(void *dst, void *src, int n);
__declspec(dllimport) int memset(void *dst, int ch, int n);
#pragma function(memcmp)
#pragma intrinsic(strlen)

__forceinline char tolower_pure(char a) {
	return a | 32;
}

static char *hextomem(char* hexstr, int *len) {
	char *mem, *src, *dst;
	int length = *len;
	char hi, lo;
	int i;
	src = hexstr;
	if (src[0] == '0' && tolower_pure(src[1]) == 'x') {
		src += 2;
		length -= 2;
	}
	length = length >> 1;
	*len = length;
	mem = malloc(length);
	dst = mem;
	while (length-- > 0) {
		hi = *src++;
		hi = hi <= '9' ? hi - '0' : tolower_pure(hi) - ('a' - 10);
		lo = *src++;
		lo = lo <= '9' ? lo - '0' : tolower_pure(lo) - ('a' - 10);
		*dst++ = (hi << 4) | lo;
	}
	return mem;
}

#define USE_VERBOSE
#undef USE_VERBOSE

int main (int argc, char **argv) {
	HANDLE hFile;
	HANDLE hMap;
	LARGE_INTEGER sizeFile;
	LPVOID pMapped;
	int i, j, len, newlen;
	BYTE *ptr;
	char *oldstr = NULL, *newstr = NULL;
	char ch;
	char flags = 0;
#define FLAG_HEX 1
#define FLAG_ALL 2
#define FLAG_OFFSET 4
#define FLAG_VERBOSE 64

	if (argc<4) return -1;

	i = 1;
	while(argv[i][0] == '-' | argv[i][0] == '/') {
		j = 1;
		while(1) {
			ch = argv[i][j];
			if (ch == 0) break;
			if (tolower_pure(ch) == 'h') flags |= FLAG_HEX;
			if (tolower_pure(ch) == 'a') flags |= FLAG_ALL;
			if (tolower_pure(ch) == 'o') flags |= FLAG_OFFSET;
#ifdef USE_VERBOSE
			if (tolower_pure(ch) == 'v') flags |= FLAG_VERBOSE;
#endif
			j++;
		}
		i++;
	}

	hFile = CreateFile(argv[i], (GENERIC_READ | GENERIC_WRITE), 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (!hFile) return -1;

	if (!GetFileSizeEx(hFile, &sizeFile)) goto MapFail;

	hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (!hMap) {
MapFail:
		CloseHandle(hFile);
		return -1;
	}

	pMapped = MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, sizeFile.LowPart);
	if (!pMapped) {
MapViewFail:
		CloseHandle(hMap);
		goto MapFail;
	}

	ptr = pMapped;
	i++;

	if (!(FLAG_OFFSET & flags)) {
		len = strlen(argv[i]);
		if (!(FLAG_HEX & flags)) oldstr = argv[i];
		else oldstr = hextomem(argv[i], &len);
	} else {
		len = strtol(argv[i], NULL, 0);
	}

	newlen = strlen(argv[i+1]);
	if (!(FLAG_HEX & flags)) {
		if (!(FLAG_OFFSET & flags) && len < newlen) goto newtoolong;
		newstr = malloc(len);
		memset(newstr, 0, len);
		memcpy(newstr, argv[i+1], newlen);
	} else {
		newstr = hextomem(argv[i+1], &newlen);
		if (!(FLAG_OFFSET & flags) && len != newlen) goto replaceend;
	}
	

#ifdef USE_VERBOSE
	if (FLAG_VERBOSE & flags && !(FLAG_OFFSET & flags) && !(FLAG_HEX & flags)) {
		char tmp[32];
		static const char fmt[9] = "src is:%.";
		memcpy(tmp, fmt, 9);
		sprintf(&tmp[9], "%ds\n", len);
		printf(tmp, oldstr);
		*((DWORD*)tmp) = MAKELONG(MAKEWORD('d','s'), MAKEWORD('t',' '));
		printf(tmp, newstr);
		printf("len is:%d\n", len);
	}
#endif

	if (!(FLAG_OFFSET & flags)) for (i=0; i<sizeFile.LowPart; i++) {
		if (i+len<=sizeFile.LowPart && !memcmp((ptr+i), oldstr, len)) {
#ifdef USE_VERBOSE
			if (FLAG_VERBOSE & flags) {
				printf("pattern found at offset: 0x%08X\n", i);
			}
#endif
			/* If not in Hex mode, replace only when string matches */
			if (FLAG_HEX & flags || i+len==sizeFile.LowPart || *(ptr+i+len)==0) {
				memcpy(ptr+i, newstr, len);
			}
			if (FLAG_ALL & flags)
				continue;
			goto replaceend;
		}
	} else {
		memcpy(ptr+len, newstr, newlen);
	}

replaceend:
	free(newstr);
newtoolong:
	if (FLAG_HEX & flags) free(oldstr);
	UnmapViewOfFile(pMapped);
	CloseHandle(hMap);
	return 0;
}