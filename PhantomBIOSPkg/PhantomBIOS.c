#include <Uefi.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi.h
#include <Library/UefiLib.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/UefiLib.h
#include <Library/UefiBootServicesTableLib.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/UefiBootServicesTableLib.h
#include <Library/DevicePathLib.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/DevicePathLib.h
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleFileSystem.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/SimpleFileSystem.h
#include <Protocol/LoadedImage.h>	// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/LoadedImage.h
#include <Library/PrintLib.h>

// Incluir las cabeceras de las funciones
#include "Functions/Files.h"
#include "Functions/PartitionsRead.h"
#include "Functions/WritePartitions.h"
#include "Functions/FileExist.h"
#include "Functions/OverwriteFile.h"

EFI_STATUS 
EFIAPI 
UefiMain ( 
    IN EFI_HANDLE        ImageHandle, 
    IN EFI_SYSTEM_TABLE  *SystemTable	// https://uefi.org/specs/UEFI/2.10/04_EFI_System_Table.html
) { 
	EFI_STATUS         Status;
	EFI_INPUT_KEY 	   Key;
	CHAR16 *FileName = L"\\EFI\\Boot\\output.txt";
        CHAR16 *OverwrittenFile = L"\\EFI\\Microsoft\\Boot\\OverwrittenFile.txt";
        CHAR8 *FileContent = "Hello Ismael, UEFI World!\n"; 
        CHAR8 *OverwrittenContent = "Overwritten Content -> Ransom (?)\n";
        CHAR8 Buffer[256];
        UINTN BufferSize = sizeof(Buffer);
        EFI_DEVICE_PATH_PROTOCOL *DevicePath; 
        EFI_HANDLE NewImageHandle = NULL;
	      EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
        EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
        EFI_FILE_PROTOCOL *Root;
        CHAR16 *AppFileName = L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi";






// https://uefi.org/specs/UEFI/2.9_A/12_Protocols_Console_Support.html#efi-simple-text-input-protocol-readkeystroke

// Loop until a key is pressed
    Print(L"Please select an option:\n1) Launch Windows\n2)Check if file exists, then read it -> If not, create it\n3)Overwrite file\n4)List partitions\n5)Overwrite partitions\n ");
    while ((Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY) {
        // Stall for a short period to avoid busy-waiting
        SystemTable->BootServices->Stall(100000); // 100ms delay
    }

if (EFI_ERROR(Status)) {
	Print(L"Error reading user input.\n");
	return Status;
}

// Check which key was pressed and respond accordingly
if (Key.UnicodeChar == L'1')
{
	Print(L"\nYou pressed '1'.\n");
	SystemTable->BootServices->Stall(100000); // 100ms delay
    // Obtener el protocolo LoadedImage para obtener la ruta del dispositivo 
     Status = gBS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&LoadedImage); 	// https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html#efi-boot-services-handleprotocol
     if (EFI_ERROR(Status)) { 
        Print(L"Error al obtener LoadedImageProtocol: %r\n", Status); 
        return Status; 
    } 

    // Obtener el protocolo de sistema de archivos 
     Status = gBS->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&SimpleFileSystem); 	// https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html#efi-boot-services-handleprotocol
     if (EFI_ERROR(Status)) { 
       Print(L"Error al obtener SimpleFileSystemProtocol: %r\n", Status); 
        return Status; 
    } 

    // Abrir la raíz del sistema de archivos 
     Status = SimpleFileSystem->OpenVolume(SimpleFileSystem, &Root); 
     if (EFI_ERROR(Status)) { 
        Print(L"Error al abrir la raíz del sistema de archivos: %r\n", Status); 
        return Status; 
     } 

    // Construir el Device Path para el archivo a cargar 
     DevicePath = FileDevicePath(LoadedImage->DeviceHandle, AppFileName); 	
     if (DevicePath == NULL) { 
        Print(L"Error al construir el Device Path para %s\n", AppFileName); 
        return EFI_INVALID_PARAMETER; 
     } 

    // Cargar la imagen de la aplicación en memoria 
     Status = gBS->LoadImage(FALSE, ImageHandle, DevicePath, NULL, 0, &NewImageHandle); 	// https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html#efi-boot-services-loadimage
     if (EFI_ERROR(Status)) { 
        Print(L"Error al cargar la imagen de la aplicación: %r\n", Status); 
        return Status; 
    } 

    //Iniciar la aplicación cargada 
     Status = gBS->StartImage(NewImageHandle, NULL, NULL); 	// https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html#efi-boot-services-startimage
     if (EFI_ERROR(Status)) { 
        Print(L"Error al iniciar la aplicación cargada: %r\n", Status); 
     } 
}
else if (Key.UnicodeChar == L'2')
{
	Print(L"\nYou pressed '2'.\n");
	SystemTable->BootServices->Stall(100000); // 100ms delay

    if (FileExist(FileName)){ 		// Llamada a la función FileExist
	    // Llamar a la función ReadFile
	    Status = ReadFile(FileName, (UINT8*)Buffer, &BufferSize);
	    if (EFI_ERROR(Status)) {
	        return Status;
	    }
    } else {

        // Llamar a la función CreateFile
     Status = CreateFile(FileName, (UINT8*)FileContent, AsciiStrLen(FileContent) * sizeof(CHAR8));
     if (EFI_ERROR(Status)) {
         return Status;
    }
    }
}
else if (Key.UnicodeChar == L'3')
{
	Print(L"\nYou pressed '3'.\n");
	SystemTable->BootServices->Stall(100000); // 100ms delay

	    // Llamar a la función ReadFile
	    Status = ReadFile(OverwrittenFile, (UINT8*)Buffer, &BufferSize);
	    if (EFI_ERROR(Status)) {
	        return Status;
	    }

    // Llamar a la función OverwriteFile
     Status = CreateFile(OverwrittenFile, (UINT8*)OverwrittenContent, AsciiStrLen(OverwrittenContent) * sizeof(CHAR8));
     if (EFI_ERROR(Status)) {
         return Status;
    }

	    // Llamar a la función ReadFile
	    Status = ReadFile(OverwrittenFile, (UINT8*)Buffer, &BufferSize);
	    if (EFI_ERROR(Status)) {
	        return Status;
	    }


}
else if (Key.UnicodeChar == L'4')
{
	Print(L"\nYou pressed '4'.\n");
	SystemTable->BootServices->Stall(100000); // 100ms delay
	// Llamar a la función ListPartitionsRead
    Status = ListPartitionsRead();
    if (EFI_ERROR(Status)) {
        return Status;
    }

}
else if (Key.UnicodeChar == L'5')
{
	Print(L"\nYou pressed '5'.\n");
	SystemTable->BootServices->Stall(100000); // 100ms delay
	// Llamar a la función WritePartitions

  Status = WritePartitions();
    if (EFI_ERROR(Status)) {
        return Status;
   }
}
else
{
	Print(L"\nYou pressed a different key: %c\n", Key.UnicodeChar);
}

// Pause
SystemTable->BootServices->Stall(3000000);    
    
	return EFI_SUCCESS; 
}
