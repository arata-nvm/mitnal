#pragma once
#include <Uefi.h>

EFI_STATUS InitConsole();

void EFIAPI PrintUtf16(IN CHAR16 *Format, ...);
