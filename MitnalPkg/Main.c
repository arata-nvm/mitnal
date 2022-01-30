#include "Common.h"
#include "Http.h"
#include "Twitter.h"
#include <Uefi.h>

EFI_STATUS Maximize(IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut) {
  EFI_STATUS Status;

  UINTN MaxMode = 0, MaxColumns = 0, MaxRows = 0;
  UINTN Columns, Rows;

  Print(L"%d, %d\n", ConOut->Mode->Mode, ConOut->Mode->MaxMode);
  for (UINTN i = 0; i < ConOut->Mode->MaxMode; i++) {
    Status = ConOut->QueryMode(ConOut, i, &Columns, &Rows);
    if (Status != EFI_SUCCESS) {
      continue;
    }

    if (Columns > MaxColumns) {
      MaxColumns = Columns;
      MaxMode = i;
    }
    if (Rows > MaxRows) {
      MaxRows = Rows;
      MaxMode = i;
    }
  }

  Status = ConOut->SetMode(ConOut, MaxMode);
  HANDLE_ERROR(Status)

  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
  EFI_STATUS Status;

  Status = Maximize(SystemTable->ConOut);
  HANDLE_ERROR(Status);

  Status = InitHttpProtocol();
  HANDLE_ERROR(Status)

  // Status = Tweet("Hello from UEFI Application.");

  TWEET *Tweets;
  Status = gBS->AllocatePool(
      EfiBootServicesData,
      sizeof(TWEET) * 32,
      (VOID **)&Tweets);
  HANDLE_ERROR(Status)
  ZeroMem(Tweets, sizeof(TWEET) * 32);

  UINTN TweetCount;
  Status = HomeTimeline(Tweets, &TweetCount);
  HANDLE_ERROR(Status)

  Print(L"%d tweets fetched.\n", TweetCount);

  for (UINTN i = 0; i < TweetCount; i++) {
    Print(L"[%a] <%a> %a\n", Tweets[i].CreatedAt, Tweets[i].UserName, Tweets[i].Text);
  }

  gBS->FreePool(Tweets);

  return EFI_SUCCESS;
}
