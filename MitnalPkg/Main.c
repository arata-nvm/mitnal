#include "Common.h"
#include "Http.h"
#include "Twitter.h"
#include <Uefi.h>

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
  EFI_STATUS Status;

  Status = InitHttpProtocol();
  HANDLE_ERROR(Status)

  Status = Tweet("Hello from UEFI Application.");
  HANDLE_ERROR(Status)

  return EFI_SUCCESS;
}
