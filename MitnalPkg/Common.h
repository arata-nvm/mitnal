#pragma once
#include "Console.h"
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>

#define HANDLE_ERROR(status)                                         \
  if (EFI_ERROR(status)) {                                           \
    PrintUtf16(L"error: %r at %a:%d\n", Status, __FILE__, __LINE__); \
    return status;                                                   \
  }

#define false (0 != 0)

typedef BOOLEAN boolean;
#ifndef __cplusplus
typedef BOOLEAN bool;
#endif
typedef CHAR8 byte;
typedef UINTN size_t;
typedef UINT8 uint8_t;
typedef UINT16 uint16_t;
typedef INT16 int16_t;
typedef UINT64 uintmax_t;

// TODO
typedef __builtin_va_list VA_LIST2;
#define VA_START2(Marker, Parameter) __builtin_va_start(Marker, Parameter)
#define VA_END2(Marker) __builtin_va_end(Marker)

extern EFI_BOOT_SERVICES *gBS;

VOID UrlEncode(CHAR8 *Str, CHAR8 *EncodedStr);

VOID *malloc(UINTN Size);

VOID free(VOID *Buffer);

VOID memcpy(VOID *Dst, const VOID *Src, UINTN Len);

VOID memset(VOID *Dst, const UINT8 Value, UINTN Len);

uintmax_t strtoumax(const char *nptr, char **endptr, int base);
