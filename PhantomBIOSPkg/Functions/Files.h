#ifndef CREATE_FILE_H
#define CREATE_FILE_H

#include <Uefi.h>
#include <Library/UefiLib.h>


EFI_STATUS
CreateFile (
    CHAR16 *FilePath,
    UINT8 *Content,
    UINTN ContentSize
);


EFI_STATUS
ReadFile (
    CHAR16 *FilePath,
	UINT8 *Buffer,
    UINTN *BufferSize
);


#endif // CREATE_FILE_H
