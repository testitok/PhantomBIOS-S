#include <Uefi.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi.h
#include <Library/UefiLib.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/UefiLib.h
#include <Library/UefiBootServicesTableLib.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/UefiBootServicesTableLib.h
#include <Library/DevicePathLib.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/DevicePathLib.h
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleFileSystem.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/SimpleFileSystem.h
#include <Protocol/LoadedImage.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/LoadedImage.h


// Función para crear un archivo y escribir datos en él
EFI_STATUS
CreateFile (
    CHAR16 *FilePath,
    UINT8 *Content,
    UINTN ContentSize
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
        return Status;
    }

    // Obtener el protocolo de sistema de archivos
    Status = gBS->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&SimpleFileSystem);	// https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html#efi-boot-services-handleprotocol
    if (EFI_ERROR(Status)) {
        Print(L"Error al obtener SimpleFileSystemProtocol: %r\n", Status);
        return Status;
    }

    // Abrir la raíz del sistema de archivos
    Status = SimpleFileSystem->OpenVolume(SimpleFileSystem, &Root);	// https://uefi.org/specs/UEFI/2.10/13_Protocols_Media_Access.html#efi-simple-file-system-protocol-openvolume
    if (EFI_ERROR(Status)) {
        Print(L"Error al abrir la raíz del sistema de archivos: %r\n", Status);
        return Status;
    }

    // Crear el archivo
    Status = Root->Open(Root, &File, FilePath, EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, 0);	// https://uefi.org/specs/UEFI/2.10/13_Protocols_Media_Access.html#efi-file-protocol-open
    if (EFI_ERROR(Status)) {
        Print(L"Error al crear el archivo %s: %r\n", FilePath, Status);
        Root->Close(Root);
        return Status;
    }

    // Escribir en el archivo
    Status = File->Write(File, &ContentSize, Content);
    if (EFI_ERROR(Status)) {
        Print(L"Error al escribir el archivo: %r\n", Status);
		
		// Cerrar el archivo y la raíz del sistema de archivos
        File->Close(File);
        Root->Close(Root);
        return Status;
    }
	else{
		// Print(L"Archivo creado y escrito exitosamente: %s\n", FilePath);
		
		// Cerrar el archivo y la raíz del sistema de archivos
		File->Close(File);
		Root->Close(Root);
	}

    return EFI_SUCCESS;
}


// Función para leer datos de un archivo
EFI_STATUS
ReadFile (
    CHAR16 *FilePath,
	UINT8 *Buffer,
    UINTN *BufferSize
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
        return Status;
    }

    // Obtener el protocolo de sistema de archivos
    Status = gBS->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&SimpleFileSystem);	// https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html#efi-boot-services-handleprotocol
    if (EFI_ERROR(Status)) {
        Print(L"Error al obtener SimpleFileSystemProtocol: %r\n", Status);
        return Status;
    }

    // Abrir la raíz del sistema de archivos
    Status = SimpleFileSystem->OpenVolume(SimpleFileSystem, &Root);	// https://uefi.org/specs/UEFI/2.10/13_Protocols_Media_Access.html#efi-simple-file-system-protocol-openvolume
    if (EFI_ERROR(Status)) {
        Print(L"Error al abrir la raíz del sistema de archivos: %r\n", Status);
        return Status;
    }

    // Abrir el archivo para leerlo
    Status = Root->Open(Root, &File, FilePath, EFI_FILE_MODE_READ, 0);	// https://uefi.org/specs/UEFI/2.10/13_Protocols_Media_Access.html#efi-file-protocol-open
    if (EFI_ERROR(Status)) {
        Print(L"Error al abrir el archivo %s para lectura: %r\n", FilePath, Status);
        Root->Close(Root);
        return Status;
    }

    // Leer el archivo
    Status = File->Read(File, BufferSize, Buffer);
    if (EFI_ERROR(Status)) {
        Print(L"Error al leer el archivo: %r\n", Status);
    } else {
        // Asegurarse de que el buffer está terminado con NULL
        Buffer[*BufferSize / sizeof(CHAR8)] = '\0';
        Print(L"Contenido del archivo %s:\n%a\n", FilePath, Buffer);
    }

    // Cerrar el archivo y la raíz del sistema de archivos
    File->Close(File);
    Root->Close(Root);
    return Status;
}
