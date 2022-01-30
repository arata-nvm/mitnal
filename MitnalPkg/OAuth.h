#pragma once
#include <Uefi.h>

EFI_STATUS GenerateOAuthHeader(IN CHAR8 *Method, IN CHAR8 *Url, IN CHAR8 *Query, OUT CHAR8 *Buffer, IN UINTN BufferSize);
