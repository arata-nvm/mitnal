#include "Http.h"
#include "Common.h"
#include "Console.h"
#include <Library/BaseMemoryLib.h>
#include <Protocol/Http.h>
#include <Protocol/ServiceBinding.h>
#include <Uefi.h>

static EFI_HTTP_PROTOCOL *gHttpProtocol;

static BOOLEAN gRequestCallbackComplete = FALSE;
static BOOLEAN gResponseCallbackComplete = FALSE;

static VOID EFIAPI RequestCallback(IN EFI_EVENT Event, IN VOID *Context) {
  gRequestCallbackComplete = TRUE;
}

static VOID EFIAPI ResponseCallback(IN EFI_EVENT Event, IN VOID *Context) {
  gResponseCallbackComplete = TRUE;
}

EFI_STATUS InitHttpProtocol() {
  EFI_STATUS Status;

  EFI_SERVICE_BINDING_PROTOCOL *ServiceBinding;
  Status = gBS->LocateProtocol(
      &gEfiHttpServiceBindingProtocolGuid,
      NULL,
      (VOID **)&ServiceBinding);
  HANDLE_ERROR(Status)

  EFI_HANDLE *Handle = NULL;
  Status = ServiceBinding->CreateChild(ServiceBinding, (VOID **)&Handle);
  HANDLE_ERROR(Status)

  Status = gBS->HandleProtocol(Handle, &gEfiHttpProtocolGuid, (VOID **)&gHttpProtocol);
  HANDLE_ERROR(Status)

  EFI_HTTP_CONFIG_DATA ConfigData;
  ConfigData.HttpVersion = HttpVersion11;
  ConfigData.TimeOutMillisec = 0;
  ConfigData.LocalAddressIsIPv6 = FALSE;

  EFI_HTTPv4_ACCESS_POINT Ipv4Node;
  ZeroMem(&Ipv4Node, sizeof(Ipv4Node));
  Ipv4Node.UseDefaultAddress = TRUE;
  ConfigData.AccessPoint.IPv4Node = &Ipv4Node;

  Status = gHttpProtocol->Configure(gHttpProtocol, &ConfigData);
  HANDLE_ERROR(Status)

  return EFI_SUCCESS;
}

EFI_STATUS SendRequest(IN HTTP_REQUEST_CONTEXT *Context) {
  EFI_STATUS Status;

  EFI_HTTP_REQUEST_DATA RequestData;
  RequestData.Method = Context->Method;
  RequestData.Url = Context->Url;

  EFI_HTTP_MESSAGE RequestMessage;
  RequestMessage.Data.Request = &RequestData;
  RequestMessage.HeaderCount = Context->HeaderCount;
  RequestMessage.Headers = Context->Headers;
  RequestMessage.BodyLength = Context->BodyLength;
  RequestMessage.Body = Context->Body;

  EFI_HTTP_TOKEN RequestToken;
  RequestToken.Event = NULL;
  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      RequestCallback,
      NULL,
      &RequestToken.Event);
  HANDLE_ERROR(Status)
  RequestToken.Status = EFI_SUCCESS;
  RequestToken.Message = &RequestMessage;

  gRequestCallbackComplete = FALSE;
  Status = gHttpProtocol->Request(gHttpProtocol, &RequestToken);
  HANDLE_ERROR(Status)
  while (!gRequestCallbackComplete)
    ;

  return EFI_SUCCESS;
}

EFI_STATUS ReceiveResponse(OUT UINT8 *Buffer, IN OUT UINTN *BufferSize) {
  EFI_STATUS Status;

  UINT8 *TempBuffer;
  Status = gBS->AllocatePool(
      EfiBootServicesData,
      0x10000,
      (VOID **)&TempBuffer);
  HANDLE_ERROR(Status)
  ZeroMem(TempBuffer, sizeof(TempBuffer));

  EFI_HTTP_RESPONSE_DATA ResponseData;
  ResponseData.StatusCode = HTTP_STATUS_UNSUPPORTED_STATUS;

  EFI_HTTP_MESSAGE ResponseMessage;
  ResponseMessage.Data.Response = &ResponseData;
  ResponseMessage.HeaderCount = 0;
  ResponseMessage.Headers = NULL;
  ResponseMessage.BodyLength = sizeof(TempBuffer);
  ResponseMessage.Body = TempBuffer;

  EFI_HTTP_TOKEN ResponseToken;
  ResponseToken.Event = NULL;
  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      ResponseCallback,
      NULL,
      &ResponseToken.Event);
  HANDLE_ERROR(Status)
  ResponseToken.Status = EFI_SUCCESS;
  ResponseToken.Message = &ResponseMessage;

  gResponseCallbackComplete = FALSE;
  Status = gHttpProtocol->Response(gHttpProtocol, &ResponseToken);
  HANDLE_ERROR(Status)
  while (!gResponseCallbackComplete)
    ;

  for (UINTN i = 0; i < ResponseMessage.HeaderCount; i++) {
    if (!AsciiStrCmp(ResponseMessage.Headers[i].FieldName, "content-length")) {
      *BufferSize = AsciiStrDecimalToUintn(ResponseMessage.Headers[i].FieldValue);
    }
  }

  UINTN ContentDownloaded = ResponseMessage.BodyLength;
  CopyMem(Buffer, TempBuffer, ResponseMessage.BodyLength);

  while (ContentDownloaded < *BufferSize) {
    ResponseMessage.Data.Response = NULL;
    if (ResponseMessage.Headers != NULL) {
      gBS->FreePool(ResponseMessage.Headers);
    }
    ResponseMessage.HeaderCount = 0;
    ResponseMessage.BodyLength = sizeof(TempBuffer);
    ZeroMem(TempBuffer, sizeof(TempBuffer));

    gResponseCallbackComplete = FALSE;
    Status = gHttpProtocol->Response(gHttpProtocol, &ResponseToken);
    HANDLE_ERROR(Status)
    while (!gResponseCallbackComplete)
      ;

    CopyMem(Buffer + ContentDownloaded, TempBuffer, ResponseMessage.BodyLength);
    ContentDownloaded += ResponseMessage.BodyLength;
  }

  gBS->FreePool(TempBuffer);

  return EFI_SUCCESS;
}
