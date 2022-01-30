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

EFI_STATUS PostTweet(IN CHAR16 *Content) {
  EFI_STATUS Status;
  CHAR8 Content8[256];

  UnicodeStrToAsciiStrS(Content, Content8, sizeof(Content8));
  Status = Tweet(Content8);
  HANDLE_ERROR(Status)

  return EFI_SUCCESS;
}

EFI_STATUS ShowTimeline() {
  EFI_STATUS Status;

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

  for (UINTN i = TweetCount - 1; i != 0; i--) {
    Print(L"[%a] <%a> %a\n", Tweets[i].CreatedAt, Tweets[i].UserName, Tweets[i].Text);
  }

  gBS->FreePool(Tweets);

  return EFI_SUCCESS;
}

EFI_STATUS ExecuteCommand(IN CHAR16 *Command) {
  if (!StrCmp(Command, L"home")) {
    return ShowTimeline();
  } else {
    CHAR16 *Arg = StrStr(Command, L" ") + 1;
    *(Arg - 1) = 0;

    if (!StrCmp(Command, L"tweet")) {
      return PostTweet(Arg);
    } else {
      Print(L"command not found\n");
    }
  }

  return EFI_SUCCESS;
}

VOID ReadLine(IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn, OUT CHAR16 *Buffer, IN UINTN BufferSize) {
  UINTN i;
  for (i = 0; i < BufferSize - 1; i++) {
    EFI_INPUT_KEY Key;
    while (ConIn->ReadKeyStroke(ConIn, &Key) != EFI_SUCCESS)
      ;

    Print(L"%c", Key.UnicodeChar);
    if (Key.UnicodeChar == L'\r') {
      Print(L"\n");
      break;
    }

    Buffer[i] = Key.UnicodeChar;
  }
  Buffer[i] = 0;
}

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
  EFI_STATUS Status;

  Status = Maximize(SystemTable->ConOut);
  HANDLE_ERROR(Status);

  Status = InitHttpProtocol();
  HANDLE_ERROR(Status)

  CHAR16 Buffer[256];
  while (TRUE) {
    Print(L"> ");
    ReadLine(SystemTable->ConIn, Buffer, sizeof(Buffer));
    ExecuteCommand(Buffer);
  }

  return EFI_SUCCESS;
}
