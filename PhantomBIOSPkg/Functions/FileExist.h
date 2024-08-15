#ifndef FILE_EXIST_H
#define FILE_EXIST_H

#include <Uefi.h>
#include <Library/UefiLib.h>


BOOLEAN
FileExist (
    CHAR16 *FilePath
);

#endif // FILE_EXIST_H
