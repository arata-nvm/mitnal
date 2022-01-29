#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Protocol/Http.h>
#include <Protocol/ServiceBinding.h>
#include <Uefi.h>

#define BUFFER_SIZE 0x100000

#define HANDLE_ERROR(status)                                    \
  if (EFI_ERROR(status)) {                                      \
    Print(L"error: %r at %a:%d\n", Status, __FILE__, __LINE__); \
    return status;                                              \
  }

extern EFI_BOOT_SERVICES *gBS;
BOOLEAN gRequestCallbackComplete = FALSE;
BOOLEAN gResponseCallbackComplete = FALSE;

VOID EFIAPI RequestCallback(IN EFI_EVENT Event, IN VOID *Context) {
  gRequestCallbackComplete = TRUE;
}

VOID EFIAPI ResponseCallback(IN EFI_EVENT Event, IN VOID *Context) {
  gResponseCallbackComplete = TRUE;
}

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
  EFI_STATUS Status;

  // init http protocol

  Print(L"initialize http protocol\n");

  UINT8 *Buffer;
  Status = gBS->AllocatePool(
      EfiBootServicesData,
      BUFFER_SIZE,
      (VOID **)&Buffer);
  HANDLE_ERROR(Status)

  EFI_SERVICE_BINDING_PROTOCOL *ServiceBinding;
  Status = gBS->LocateProtocol(
      &gEfiHttpServiceBindingProtocolGuid,
      NULL,
      (VOID **)&ServiceBinding);
  HANDLE_ERROR(Status)

  EFI_HANDLE *Handle = NULL;
  Status = ServiceBinding->CreateChild(ServiceBinding, (VOID **)&Handle);
  HANDLE_ERROR(Status)

  EFI_HTTP_PROTOCOL *HttpProtocol;
  Status = gBS->HandleProtocol(Handle, &gEfiHttpProtocolGuid, (VOID **)&HttpProtocol);
  HANDLE_ERROR(Status)

  EFI_HTTP_CONFIG_DATA ConfigData;
  ConfigData.HttpVersion = HttpVersion11;
  ConfigData.TimeOutMillisec = 0;
  ConfigData.LocalAddressIsIPv6 = FALSE;

  EFI_HTTPv4_ACCESS_POINT Ipv4Node;
  ZeroMem(&Ipv4Node, sizeof(Ipv4Node));
  Ipv4Node.UseDefaultAddress = TRUE;
  ConfigData.AccessPoint.IPv4Node = &Ipv4Node;

  Status = HttpProtocol->Configure(HttpProtocol, &ConfigData);
  HANDLE_ERROR(Status)

  // send http request

  Print(L"send http request\n");

  EFI_HTTP_REQUEST_DATA RequestData;
  RequestData.Method = HttpMethodGet;
  RequestData.Url = L"http://jsonplaceholder.typicode.com/posts/1";

  EFI_HTTP_HEADER RequestHeader;
  RequestHeader.FieldName = "Host";
  RequestHeader.FieldValue = "jsonplaceholder.typicode.com";

  EFI_HTTP_MESSAGE RequestMessage;
  RequestMessage.Data.Request = &RequestData;
  RequestMessage.HeaderCount = 1;
  RequestMessage.Headers = &RequestHeader;
  RequestMessage.BodyLength = 0;
  RequestMessage.Body = NULL;

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
  Status = HttpProtocol->Request(HttpProtocol, &RequestToken);
  HANDLE_ERROR(Status)
  while (!gRequestCallbackComplete)
    ;

  // receive http response

  Print(L"receive http response\n");

  EFI_HTTP_RESPONSE_DATA ResponseData;
  ResponseData.StatusCode = HTTP_STATUS_UNSUPPORTED_STATUS;

  EFI_HTTP_MESSAGE ResponseMessage;
  ResponseMessage.Data.Response = &ResponseData;
  ResponseMessage.HeaderCount = 0;
  ResponseMessage.Headers = NULL;
  ResponseMessage.BodyLength = BUFFER_SIZE;
  ResponseMessage.Body = Buffer;

  EFI_HTTP_TOKEN ResponseToken;
  RequestToken.Event = NULL;
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
  Status = HttpProtocol->Response(HttpProtocol, &ResponseToken);
  HANDLE_ERROR(Status)
  while (!gResponseCallbackComplete)
    ;

  Print(L"%a\n", Buffer);

  Print(L"finished\n");

  return EFI_SUCCESS;
}
