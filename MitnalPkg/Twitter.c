#include "Twitter.h"
#include "Common.h"
#include "Http.h"
#include "OAuth.h"
#include "json.h"
#include <Library/PrintLib.h>

#define BUFFER_SIZE 0x100000

static CHAR8 *API_URL_TWEET = "https://api.twitter.com/1.1/statuses/update.json";
static CHAR8 *API_URL_HOME = "https://api.twitter.com/1.1/statuses/home_timeline.json";
// static CHAR8 *API_URL_TIMELINE = "https://api.twitter.com/1.1/statuses/user_timeline.json";

EFI_STATUS Tweet(IN CHAR8 *Message) {
  EFI_STATUS Status;

  CHAR8 MessageEnc[256];
  CHAR8 Query[256];
  CHAR16 Url[256];
  CHAR8 OAuthHeader[512];

  UrlEncode(Message, MessageEnc);
  AsciiSPrint(Query, sizeof(Query), "status=%a", MessageEnc);
  UnicodeSPrint(Url, sizeof(Url), L"%a?%a", API_URL_TWEET, Query);

  EFI_HTTP_HEADER RequestHeader[2];
  RequestHeader[0].FieldName = "Host";
  RequestHeader[0].FieldValue = "api.twitter.com";

  GenerateOAuthHeader("POST", API_URL_TWEET, Query, OAuthHeader, sizeof(OAuthHeader));
  RequestHeader[1].FieldName = "Authorization";
  RequestHeader[1].FieldValue = OAuthHeader;

  HTTP_REQUEST_CONTEXT Context;
  Context.Method = HttpMethodPost;
  Context.Url = Url;
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
  HANDLE_ERROR(Status);

  UINTN BufferSize = BUFFER_SIZE;
  Status = ReceiveResponse(Buffer, &BufferSize);
  HANDLE_ERROR(Status);

  gBS->FreePool(Buffer);

  return EFI_SUCCESS;
}

void ParseTweets(IN UINT8 *Buffer, IN UINTN BufferSize, OUT TWEET *Tweets, OUT UINTN *TweetCount) {
  struct json_value_s *root = json_parse(Buffer, BufferSize);
  struct json_array_s *array = json_value_as_array(root);

  UINTN i = 0;
  for (struct json_array_element_s *ae = array->start; ae != NULL; ae = ae->next) {
    struct json_object_s *object = json_value_as_object(ae->value);
    for (struct json_object_element_s *oe = object->start; oe != NULL; oe = oe->next) {
      if (!AsciiStrCmp(oe->name->string, "created_at")) {
        CHAR8 *CreatedAt = (CHAR8 *)json_value_as_string(json_extract_value(oe->value))->string;

        // TODO
        CreatedAt = CreatedAt + 4;
        CreatedAt[12] = 0;

        Tweets[i].CreatedAt = CreatedAt;
      } else if (!AsciiStrCmp(oe->name->string, "text")) {
        Tweets[i].Text = json_value_as_string(json_extract_value(oe->value))->string;
      } else if (!AsciiStrCmp(oe->name->string, "user")) {
        struct json_object_s *object = json_value_as_object(oe->value);
        for (struct json_object_element_s *oe2 = object->start; oe2 != NULL; oe2 = oe2->next) {
          if (!AsciiStrCmp(oe2->name->string, "name")) {
            Tweets[i].UserName = json_value_as_string(json_extract_value(oe2->value))->string;
          }
        }
      }
    }
    i++;
  }
  *TweetCount = i;

  free(root);
}

EFI_STATUS HomeTimeline(OUT TWEET *Tweets, OUT UINTN *TweetCount) {
  EFI_STATUS Status;

  CHAR16 Url[256];
  CHAR8 OAuthHeader[512];

  AsciiStrToUnicodeStrS(API_URL_HOME, Url, sizeof(Url));

  EFI_HTTP_HEADER RequestHeader[2];
  RequestHeader[0].FieldName = "Host";
  RequestHeader[0].FieldValue = "api.twitter.com";

  GenerateOAuthHeader("GET", API_URL_HOME, NULL, OAuthHeader, sizeof(OAuthHeader));
  RequestHeader[1].FieldName = "Authorization";
  RequestHeader[1].FieldValue = OAuthHeader;

  HTTP_REQUEST_CONTEXT Context;
  Context.Method = HttpMethodGet;
  Context.Url = Url;
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
  HANDLE_ERROR(Status);

  UINTN BufferSize = BUFFER_SIZE;
  Status = ReceiveResponse(Buffer, &BufferSize);
  HANDLE_ERROR(Status);

  ParseTweets(Buffer, BufferSize, Tweets, TweetCount);

  gBS->FreePool(Buffer);

  return EFI_SUCCESS;
}
