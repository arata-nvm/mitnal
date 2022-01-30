#pragma once
#include <Protocol/Http.h>
#include <Uefi.h>

typedef struct {
  EFI_HTTP_METHOD Method;
  CHAR16 *Url;

  EFI_HTTP_HEADER *Headers;
  UINTN HeaderCount;

  VOID *Body;
  UINTN BodyLength;
} HTTP_REQUEST_CONTEXT;

EFI_STATUS InitHttpProtocol();

EFI_STATUS SendRequest(IN HTTP_REQUEST_CONTEXT *Context);

EFI_STATUS ReceiveResponse(OUT UINT8 *Buffer, IN UINTN BufferSize);
