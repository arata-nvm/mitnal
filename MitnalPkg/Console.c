#include "Console.h"
#include "Common.h"
#include "Graphics.h"
#include <Library/PrintLib.h>
#include <Uefi.h>

static UINTN gColumn = 0;
static UINTN gRow = 0;
static UINTN gMaxColumn;
static UINTN gMaxRow;
static CHAR16 *gBuffer;

EFI_STATUS InitConsole() {
  EFI_STATUS Status;

  gMaxColumn = gHorizontalResolution / FONT_SIZE;
  gMaxRow = gVerticalResolution / FONT_SIZE;

  Status = gBS->AllocatePool(
      EfiBootServicesData,
      gMaxColumn * gMaxRow,
      (VOID **)&gBuffer);
  HANDLE_ERROR(Status);
  ZeroMem(gBuffer, gMaxColumn * gMaxRow);

  return EFI_SUCCESS;
}

// static BOOLEAN IsHalfWidth(CHAR16 Char) {
//   return Char <= 0xff;
// }

static VOID Scroll() {
  for (UINTN y = 0; y < gMaxRow - 1; y++) {
    for (UINTN x = 0; x < gMaxColumn; x++) {
      gBuffer[x + y * gMaxColumn] = gBuffer[x + (y + 1) * gMaxColumn];
      DrawChar(x * FONT_SIZE, y * FONT_SIZE, gBuffer[x + y * gMaxColumn]);
    }
  }

  for (UINTN x = 0; x < gMaxColumn; x++) {
    gBuffer[x + (gMaxRow - 1) * gMaxColumn] = 0;
    DrawChar(x * FONT_SIZE, (gMaxRow - 1) * FONT_SIZE, L' ');
  }
}

static VOID PrintChar(CHAR16 Char) {
  if (Char == L'\b') {
    if (gColumn > 0) {
      gColumn--;
      // DrawChar(gColumn, gRow, L' ');
    }
  } else if (Char == L'\r') {
    gColumn = 0;
  } else if (Char == L'\n') {
    gColumn = 0;
    gRow++;
  } else {
    gBuffer[gColumn + gRow * gMaxColumn] = Char;
    DrawChar(gColumn * FONT_SIZE, gRow * FONT_SIZE, gBuffer[gColumn + gRow * gMaxColumn]);
    gColumn++;
  }

  if (gColumn == gMaxColumn) {
    gColumn = 0;
    gRow++;
  }
  if (gRow == gMaxRow) {
    Scroll();
    gRow--;
  }
}

void EFIAPI PrintUtf16(IN CHAR16 *Format, ...) {
  VA_LIST Marker;

  VA_START(Marker, Format);

  CHAR16 Buffer[256];
  UnicodeVSPrint(Buffer, sizeof(Buffer), Format, Marker);

  for (UINTN i = 0; Buffer[i] != 0; i++) {
    PrintChar(Buffer[i]);
  }

  VA_END(Marker);
}