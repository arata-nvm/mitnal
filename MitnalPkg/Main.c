#include "Common.h"
#include "Console.h"
#include "Graphics.h"
#include "Http.h"
#include "Twitter.h"
#include <Uefi.h>

char *efontUFT8toUTF16(uint16_t *pUTF16, char *pUTF8);

VOID UTF8toUTF16(OUT CHAR16 *pUTF16, IN CHAR8 *pUTF8) {
  UINTN i = 0;
  CHAR8 *ptr = pUTF8;
  while (*ptr != 0) {
    ptr = efontUFT8toUTF16(&pUTF16[i], ptr);
    i++;
  }
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

  CHAR16 CreatedAt[256];
  CHAR16 UserName[256];
  CHAR16 Text[256];
  for (UINTN i = TweetCount - 1; i != 0; i--) {
    ZeroMem(CreatedAt, sizeof(CreatedAt));
    ZeroMem(UserName, sizeof(UserName));
    ZeroMem(Text, sizeof(Text));
    UTF8toUTF16(CreatedAt, (CHAR8 *)Tweets[i].CreatedAt);
    UTF8toUTF16(UserName, (CHAR8 *)Tweets[i].UserName);
    UTF8toUTF16(Text, (CHAR8 *)Tweets[i].Text);

    PrintUtf16(L"[%s] <%s> %s\n", CreatedAt, UserName, Text);
  }

  gBS->FreePool(Tweets);

  return EFI_SUCCESS;
}

EFI_STATUS ExecuteCommand(IN CHAR16 *Command) {
  if (!StrCmp(Command, L"home")) {
    return ShowTimeline();
  } else if (!StrCmp(Command, L"exit")) {
    return EFI_ABORTED;
  } else {
    CHAR16 *Arg = StrStr(Command, L" ") + 1;
    *(Arg - 1) = 0;

    if (!StrCmp(Command, L"tweet")) {
      return PostTweet(Arg);
    } else {
      PrintUtf16(L"command not found\n");
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

    if (Key.UnicodeChar == L'\r') {
      PrintUtf16(L"\n");
      break;
    } else if (Key.UnicodeChar == L'\b') {
      if (i == 0) {
        continue;
      }
      i--;
    }

    PrintUtf16(L"%c", Key.UnicodeChar);
    Buffer[i] = Key.UnicodeChar;
  }
  Buffer[i] = 0;
}

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
  EFI_STATUS Status;

  Status = InitGraphics(ImageHandle);
  HANDLE_ERROR(Status)

  Status = InitConsole();
  HANDLE_ERROR(Status)

  Status = InitHttpProtocol();
  HANDLE_ERROR(Status)

  CHAR16 Buffer[256];
  while (TRUE) {
    PrintUtf16(L"> ");
    ReadLine(SystemTable->ConIn, Buffer, sizeof(Buffer));
    Status = ExecuteCommand(Buffer);
    HANDLE_ERROR(Status)
  }

  return EFI_SUCCESS;
}
