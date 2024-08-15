#include <Uefi.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi.h
#include <Library/UefiLib.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/UefiLib.h
#include <Library/UefiBootServicesTableLib.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/UefiBootServicesTableLib.h
#include <Library/DevicePathLib.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/DevicePathLib.h
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>


// Función para verificar la existencia de un archivo
BOOLEAN
FileExist (
    CHAR16 *FilePath
)
{
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
    EFI_FILE_PROTOCOL *Root;
    EFI_FILE_PROTOCOL *File;

    // Obtener el protocolo LoadedImage para obtener la ruta del dispositivo
    Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&LoadedImage);	// https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html#efi-boot-services-handleprotocol
    if (EFI_ERROR(Status)) {
        Print(L"Error al obtener LoadedImageProtocol: %r\n", Status);
        return FALSE;
    }

    // Obtener el protocolo de sistema de archivos
    Status = gBS->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&SimpleFileSystem);	// https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html#efi-boot-services-handleprotocol
    if (EFI_ERROR(Status)) {
        Print(L"Error al obtener SimpleFileSystemProtocol: %r\n", Status);
        return FALSE;
    }

    // Abrir la raíz del sistema de archivos
    Status = SimpleFileSystem->OpenVolume(SimpleFileSystem, &Root);	// https://uefi.org/specs/UEFI/2.10/13_Protocols_Media_Access.html#efi-simple-file-system-protocol-openvolume
    if (EFI_ERROR(Status)) {
        Print(L"Error al abrir la raíz del sistema de archivos: %r\n", Status);
        return FALSE;
    }

    // Abrir el archivo para leerlo
    Status = Root->Open(Root, &File, FilePath, EFI_FILE_MODE_READ, 0);	// https://uefi.org/specs/UEFI/2.10/13_Protocols_Media_Access.html#efi-file-protocol-open
    if (EFI_ERROR(Status)) {
        Print(L"Error al abrir el archivo %s para lectura: %r\n", FilePath, Status);
        Root->Close(Root);
        return FALSE;
    }
    return TRUE;
}
