#ifndef OVERWRITE_FILE_H
#define OVERWRITE_FILE_H

#include <Uefi.h>
#include <Library/UefiLib.h>


EFI_STATUS
OverwriteFile (
    CHAR16 *FilePath,
    UINT8 *Content,
    UINTN ContentSize
);

#endif // OVERWRITE_FILE_H
