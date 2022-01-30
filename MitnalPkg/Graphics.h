#pragma once
#include <Uefi.h>

#define FONT_SIZE 16
#define FONT_SIZE_HALF (FONT_SIZE / 2)

extern UINTN gHorizontalResolution;
extern UINTN gVerticalResolution;

EFI_STATUS InitGraphics(IN EFI_HANDLE ImageHandle);

void DrawChar(UINTN x, UINTN y, CHAR16 Char);
