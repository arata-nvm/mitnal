#pragma once
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>

#define HANDLE_ERROR(status)                                    \
  if (EFI_ERROR(status)) {                                      \
    Print(L"error: %r at %a:%d\n", Status, __FILE__, __LINE__); \
    return status;                                              \
  }

typedef UINTN size_t;
typedef UINT64 uintmax_t;

extern EFI_BOOT_SERVICES *gBS;

VOID UrlEncode(CHAR8 *Str, CHAR8 *EncodedStr);

VOID *malloc(UINTN Size);

VOID free(VOID *Buffer);

VOID memcpy(VOID *Dst, const VOID *Src, UINTN Len);

uintmax_t strtoumax(const char *nptr, char **endptr, int base);
