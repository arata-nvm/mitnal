#include "Common.h"
#include <Uefi.h>

// ref: https://xml2rfc.tools.ietf.org/public/rfc/html/rfc3986.html#unreserved
void UrlEncode(CHAR8 *Str, CHAR8 *StrEnc) {
  static const CHAR8 HEX[] = "0123456789ABCDEF";

  UINTN StrSize = AsciiStrLen(Str);
  CHAR8 *ptr = StrEnc;
  for (UINTN i = 0; i < StrSize; i++) {
    CHAR8 c = Str[i];
    if ((0x30 <= c && c <= 0x39) || (0x41 <= c && c <= 0x5a) || (0x61 <= c && c <= 0x7a) || c == 0x2d || c == 0x2e || c == 0x5f || c == 0x7e) {
      *ptr++ = c;
      continue;
    }

    *ptr++ = '%';
    *ptr++ = HEX[c >> 4];
    *ptr++ = HEX[c & 0xf];
  }
  *ptr = 0;
}

VOID *malloc(UINTN Size) {
  VOID *Buffer;
  gBS->AllocatePool(
      EfiBootServicesData,
      Size,
      (VOID **)&Buffer);
  return Buffer;
}

VOID free(VOID *Buffer) {
  gBS->FreePool(Buffer);
}

VOID memcpy(VOID *Dst, const VOID *Src, UINTN Len) {
  CopyMem(Dst, Src, Len);
}

uintmax_t
strtoumax(const char *nptr, char **endptr, int base) {
  return 0;
}
