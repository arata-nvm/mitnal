#include "Common.h"
#include "hmac/hmac.h"
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Protocol/Timestamp.h>
#include <Uefi.h>

#include "Secrets"

EFI_STATUS HmacSha1(IN CHAR8 *Key, IN CHAR8 *Data, OUT CHAR8 *Result, OUT UINTN *ResultSize) {
  UINTN DigestSize;
  CHAR8 ResultRaw[256];

  hmac_sha1((UINT8 *)Key, AsciiStrLen(Key), (UINT8 *)Data, AsciiStrLen(Data), (UINT8 *)ResultRaw, (size_t *)&DigestSize);
  Base64Encode((UINT8 *)ResultRaw, DigestSize, Result, ResultSize);

  return EFI_SUCCESS;
}

EFI_STATUS GenerateSignature(IN CHAR8 *Method, IN CHAR8 *Url, IN CHAR8 *Query, OUT CHAR8 *Signature, IN UINTN SignatureSize) {
  CHAR8 Key[256];
  CHAR8 Base[1024];

  CHAR8 UrlEnc[256];
  CHAR8 Param[1024];
  CHAR8 ParamEnc[1024];

  AsciiSPrint(Key, sizeof(Key), "%a&%a", gConsumerSecret, gAccessTokenSecret);

  if (Query == NULL) {
    AsciiSPrint(
        Param, sizeof(Param),
        "oauth_consumer_key=%a&oauth_nonce=%a&oauth_signature_method=HMAC-SHA1&oauth_timestamp=%d&oauth_token=%a&oauth_version=1.0",
        gConsumerKey, gNonce, gTimestamp, gAccessToken);
  } else {
    AsciiSPrint(
        Param, sizeof(Param),
        "oauth_consumer_key=%a&oauth_nonce=%a&oauth_signature_method=HMAC-SHA1&oauth_timestamp=%d&oauth_token=%a&oauth_version=1.0&%a",
        gConsumerKey, gNonce, gTimestamp, gAccessToken, Query);
  }

  UrlEncode(Url, UrlEnc);
  UrlEncode(Param, ParamEnc);
  AsciiSPrint(Base, sizeof(Base), "%a&%a&%a", Method, UrlEnc, ParamEnc);

  HmacSha1(Key, Base, Signature, &SignatureSize);

  return EFI_SUCCESS;
}

EFI_STATUS GenerateOAuthHeader(IN CHAR8 *Method, IN CHAR8 *Url, IN CHAR8 *Query, OUT CHAR8 *Buffer, IN UINTN BufferSize) {
  EFI_STATUS Status;

  CHAR8 Signature[256];
  CHAR8 SignatureEnc[256];

  Status = GenerateSignature(Method, Url, Query, Signature, sizeof(Signature));
  HANDLE_ERROR(Status)

  UrlEncode(Signature, SignatureEnc);
  AsciiSPrint(
      Buffer,
      BufferSize,
      "OAuth oauth_consumer_key=\"%a\", oauth_nonce=\"%a\", oauth_signature=\"%a\", oauth_signature_method=\"HMAC-SHA1\", oauth_timestamp=\"%d\", oauth_token=\"%a\", oauth_version=\"1.0\"",
      gConsumerKey,
      gNonce,
      SignatureEnc,
      gTimestamp,
      gAccessToken);

  return EFI_SUCCESS;
}
