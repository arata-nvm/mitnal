#pragma once
#include <Library/UefiLib.h>

extern EFI_BOOT_SERVICES *gBS;

#define HANDLE_ERROR(status)                                    \
  if (EFI_ERROR(status)) {                                      \
    Print(L"error: %r at %a:%d\n", Status, __FILE__, __LINE__); \
    return status;                                              \
  }

void UrlEncode(CHAR8 *Str, CHAR8 *EncodedStr);
