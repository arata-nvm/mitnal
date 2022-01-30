#include "Twitter.h"
#include "Common.h"
#include "Http.h"
#include "OAuth.h"
#include <Library/PrintLib.h>

#define BUFFER_SIZE 0x100000

static CHAR8 *TWITTER_API_URL = "https://api.twitter.com/1.1/statuses/update.json";

EFI_STATUS Tweet(IN CHAR8 *Message) {
  EFI_STATUS Status;

  CHAR8 MessageEnc[256];
  CHAR8 Query[256];
  CHAR16 Url[256];
  CHAR8 OAuthHeader[512];

  UrlEncode(Message, MessageEnc);
  AsciiSPrint(Query, sizeof(Query), "status=%a", MessageEnc);
  UnicodeSPrint(Url, sizeof(Url), L"%a?%a", TWITTER_API_URL, Query);

  EFI_HTTP_HEADER RequestHeader[2];
  RequestHeader[0].FieldName = "Host";
  RequestHeader[0].FieldValue = "api.twitter.com";

  GenerateOAuthHeader("POST", TWITTER_API_URL, Query, OAuthHeader, sizeof(OAuthHeader));
  RequestHeader[1].FieldName = "Authorization";
  RequestHeader[1].FieldValue = OAuthHeader;

  HTTP_REQUEST_CONTEXT Context;
  Context.Method = HttpMethodPost;
  Context.Url = Url;
  Print(L"%s\n", Url);
  Context.HeaderCount = 2;
  Context.Headers = RequestHeader;
  Context.BodyLength = 0;
  Context.Body = NULL;

  Status = SendRequest(&Context);
  HANDLE_ERROR(Status);

  UINT8 *Buffer;
  Status = gBS->AllocatePool(
      EfiBootServicesData,
      BUFFER_SIZE,
      (VOID **)&Buffer);
  HANDLE_ERROR(Status)

  Status = ReceiveResponse(Buffer, BUFFER_SIZE);
  HANDLE_ERROR(Status);

  gBS->FreePool(Buffer);

  return EFI_SUCCESS;
}
