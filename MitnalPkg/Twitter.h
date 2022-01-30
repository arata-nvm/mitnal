#pragma once
#include <Uefi.h>

typedef struct {
  const CHAR8 *CreatedAt;
  const CHAR8 *UserName;
  const CHAR8 *Text;
} TWEET;

EFI_STATUS Tweet(IN CHAR8 *Message);

EFI_STATUS HomeTimeline(OUT TWEET *Tweets, OUT UINTN *TweetCount);
