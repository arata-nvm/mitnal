#include "Common.h"
#include "Console.h"
#include <Uefi.h>

#define PROGMEM
#include "font/efont.h"

static EFI_GRAPHICS_OUTPUT_PROTOCOL *gGop;
static UINT8 *gFrameBuffer;
static UINTN gFrameBufferSize;
static UINTN gPixelsPerScanLine;
UINTN gHorizontalResolution;
UINTN gVerticalResolution;

EFI_STATUS InitGraphics(IN EFI_HANDLE ImageHandle) {
  EFI_STATUS Status;

  UINTN HandleSize = 0;
  EFI_HANDLE *Handles = NULL;
  Status = gBS->LocateHandleBuffer(
      ByProtocol,
      &gEfiGraphicsOutputProtocolGuid,
      NULL,
      &HandleSize,
      &Handles);
  HANDLE_ERROR(Status)

  Status = gBS->OpenProtocol(
      Handles[0],
      &gEfiGraphicsOutputProtocolGuid,
      (VOID **)&gGop,
      ImageHandle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  HANDLE_ERROR(Status)

  gBS->FreePool(Handles);

  gFrameBuffer = (UINT8 *)gGop->Mode->FrameBufferBase;
  gFrameBufferSize = gGop->Mode->FrameBufferSize;
  gPixelsPerScanLine = gGop->Mode->Info->PixelsPerScanLine;
  gHorizontalResolution = gGop->Mode->Info->HorizontalResolution;
  gVerticalResolution = gGop->Mode->Info->VerticalResolution;

  ZeroMem(gFrameBuffer, gFrameBufferSize);

  return EFI_SUCCESS;
}

void PutPixel(UINTN x, UINTN y, BOOLEAN colored) {
  UINT8 *ptr = gFrameBuffer + 4 * (gPixelsPerScanLine * y + x);
  UINT8 c = colored ? 255 : 0;
  ptr[0] = c;
  ptr[1] = c;
  ptr[2] = c;
}

void DrawChar(UINTN x, UINTN y, CHAR16 Char) {
  UINT16 Buffer[16];
  getefontData((UINT8 *)&Buffer, Char);

  for (UINTN dy = 0; dy < 16; dy++) {
    for (UINTN dx = 0; dx < 16; dx++) {
      UINT16 FontData = (Buffer[dy] >> 8) | (Buffer[dy] << 8);
      BOOLEAN Colored = (FontData >> (15 - dx)) & 0x1;
      PutPixel(x + dx, y + dy, Colored);
    }
  }
}
