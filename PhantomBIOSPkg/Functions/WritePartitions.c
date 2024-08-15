#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>
#include <Protocol/DevicePath.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/BlockIo.h>
#include <IndustryStandard/Mbr.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/FileHandleLib.h>
#include "Functions/Files.h"


// Estructura para almacenar información de particiones soportadas
typedef struct {
    CHAR16 *DevicePathStr;
    UINT64 PartitionSize;
    CHAR16 **Files;
    UINTN FileCount;
} PartitionInfo;


// Estructura para almacenar información de particiones no soportadas
typedef struct {
    CHAR16 *DevicePathStr;
    UINT64 PartitionSize;
} UnsupportedPartitionInfo;

// Escribir en partición que no soporta el sistema de archivos simple
EFI_STATUS WriteDataToUnsupportedPartition(EFI_BLOCK_IO_PROTOCOL *BlockIo, UINT64 Lba, UINTN Size, UINT8 Pattern) {
    EFI_STATUS Status;    VOID *Buffer;
    // Asignar memoria para el buffer
    Buffer = AllocatePool(Size);    if (Buffer == NULL) {
        Print(L"Error al asignar memoria para el buffer\n");        return EFI_OUT_OF_RESOURCES;
    }
    // Rellenar el buffer con el patrón fijo    
	SetMem(Buffer, Size, Pattern);
    // Escribir datos en el bloque
    Status = BlockIo->WriteBlocks(BlockIo, BlockIo->Media->MediaId, Lba, Size, Buffer);    
	if (EFI_ERROR(Status)) {
        	Print(L"Error al escribir bloques: %r\n", Status);        
		FreePool(Buffer);
        return Status;    }
    	// Liberar el buffer
	FreePool(Buffer);   
	 return EFI_SUCCESS;
}


// Leer datos directamente de una partición que no soporta el sistema de archivos simple
EFI_STATUS ReadRawDataWrite(EFI_BLOCK_IO_PROTOCOL *BlockIo, UINT64 Lba, UINTN BufferSize, CHAR16 *FilePath) {
    EFI_STATUS Status;
    VOID *Buffer;

    // Asignar memoria para el buffer
    Buffer = AllocatePool(BufferSize);
    if (Buffer == NULL) {
        Print(L"Error al asignar memoria para el buffer\n");
        return EFI_OUT_OF_RESOURCES;
    }

    // Leer datos del bloque
    Status = BlockIo->ReadBlocks(BlockIo, BlockIo->Media->MediaId, Lba, BufferSize, Buffer);
    if (EFI_ERROR(Status)) {
        Print(L"Error al leer bloques: %r\n", Status);
        FreePool(Buffer);
        return Status;
    }

    // Mostrar datos en formato hexadecimal
    //Print(L"Datos leídos desde LBA %llu:\n", Lba);
    //for (UINTN i = 0; i < BufferSize; i++) {
    //    Print(L"%02x ", ((UINT8*)Buffer)[i]);
    //    if ((i + 1) % 16 == 0) {
    //        Print(L"\n");
    //    }
    //}
    //Print(L"\n");

    // Volcar los datos leídos a un archivo
    Status = CreateFile(FilePath, Buffer, BufferSize);
    if (EFI_ERROR(Status)) {
        Print(L"Error al crear el archivo: %r\n", Status);
    }

    // Liberar el buffer
    FreePool(Buffer);
    return EFI_SUCCESS;
}


// Listar ficheros y directorios raíz de una partición
EFI_STATUS ListDirWrite(EFI_FILE_PROTOCOL *Dir, CHAR16 ***FileList, UINTN *FileCount) {
    EFI_STATUS Status;
    EFI_FILE_INFO *FileInfo;
    UINTN FileInfoSize;
    UINTN BufferSize;
    CHAR16 *FileName;

    *FileList = NULL;
    *FileCount = 0;

    // Definir el tamaño del buffer para EFI_FILE_INFO
    BufferSize = SIZE_OF_EFI_FILE_INFO + 1024;
    FileInfo = AllocatePool(BufferSize);
    if (FileInfo == NULL) {
        Print(L"Error al asignar memoria para FileInfo\n");
        return EFI_OUT_OF_RESOURCES;
    }

    while (TRUE) {
        FileInfoSize = BufferSize;
        Status = Dir->Read(Dir, &FileInfoSize, FileInfo);
        if (EFI_ERROR(Status) || FileInfoSize == 0) {
            break;
        }

        FileName = AllocateCopyPool(StrSize(FileInfo->FileName), FileInfo->FileName);
        if (FileName == NULL) {
            FreePool(FileInfo);
            return EFI_OUT_OF_RESOURCES;
        }

        // Redimensionar el buffer para la lista de archivos
        *FileList = ReallocatePool(
            *FileCount * sizeof(CHAR16 *),
            (*FileCount + 1) * sizeof(CHAR16 *),
            *FileList
        );

        if (*FileList == NULL) {
            FreePool(FileName);
            FreePool(FileInfo);
            return EFI_OUT_OF_RESOURCES;
        }

        (*FileList)[*FileCount] = FileName;
        (*FileCount)++;
    }

    FreePool(FileInfo);
    return EFI_SUCCESS;
}


// Función principal para listar particiones y leer datos de una partición no soportada
EFI_STATUS WritePartitions() {
    EFI_STATUS Status;
    EFI_HANDLE *HandleBuffer;
    UINTN HandleCount;
    EFI_BLOCK_IO_PROTOCOL *BlockIo;
    EFI_DEVICE_PATH_PROTOCOL *DevicePath;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
    EFI_FILE_PROTOCOL *Root;
    UINTN i;

    PartitionInfo *SupportedPartitions = NULL;
    UINTN SupportedCount = 0;

    UnsupportedPartitionInfo *UnsupportedPartitions = NULL;
    UINTN UnsupportedCount = 0;

    // Localizar todos los manejadores que soportan el protocolo Block IO
    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
    if (EFI_ERROR(Status)) {
        Print(L"No se pudo localizar ningún manejador de Block IO: %r\n", Status);
        return Status;
    }

    // Iterar a través de cada manejador y comprobar si soporta el protocolo Simple File System
    for (i = 0; i < HandleCount; i++) {
        // Obtener el protocolo Block IO
        Status = gBS->HandleProtocol(HandleBuffer[i], &gEfiBlockIoProtocolGuid, (VOID**)&BlockIo);
        if (EFI_ERROR(Status)) {
            continue;
        }

        // Obtener la ruta del dispositivo para el manejador
        Status = gBS->HandleProtocol(HandleBuffer[i], &gEfiDevicePathProtocolGuid, (VOID**)&DevicePath);
        if (EFI_ERROR(Status)) {
            continue;
        }

        // Obtener el tamaño de la partición
        UINT64 PartitionSize = BlockIo->Media->BlockSize * (BlockIo->Media->LastBlock + 1);

        // Comprobar si el manejador soporta el protocolo Simple File System
        Status = gBS->HandleProtocol(HandleBuffer[i], &gEfiSimpleFileSystemProtocolGuid, (VOID**)&SimpleFileSystem);
        if (!EFI_ERROR(Status)) {
            // Abrir el volumen para obtener el directorio raíz
            Status = SimpleFileSystem->OpenVolume(SimpleFileSystem, &Root);
            if (EFI_ERROR(Status)) {
                continue;
            }

            CHAR16 **FileList = NULL;
            UINTN FileCount = 0;

            // Listar el contenido del directorio raíz
            Status = ListDirWrite(Root, &FileList, &FileCount);
            if (EFI_ERROR(Status)) {
                Root->Close(Root);
                continue;
            }

            // Cerrar el directorio raíz
            Root->Close(Root);

            CHAR16 *DevicePathStr = ConvertDevicePathToText(DevicePath, FALSE, FALSE);
            if (DevicePathStr == NULL) {
                DevicePathStr = L"Desconocida";
            }

            // Redimensionar el buffer para particiones soportadas
            SupportedPartitions = ReallocatePool(
                SupportedCount * sizeof(PartitionInfo),
                (SupportedCount + 1) * sizeof(PartitionInfo),
                SupportedPartitions
            );

            if (SupportedPartitions == NULL) {
                return EFI_OUT_OF_RESOURCES;
            }

            SupportedPartitions[SupportedCount].DevicePathStr = DevicePathStr;
            SupportedPartitions[SupportedCount].PartitionSize = PartitionSize;
            SupportedPartitions[SupportedCount].Files = FileList;
            SupportedPartitions[SupportedCount].FileCount = FileCount;
            SupportedCount++;
        } else {
            CHAR16 *DevicePathStr = ConvertDevicePathToText(DevicePath, FALSE, FALSE);
            if (DevicePathStr == NULL) {
                DevicePathStr = L"Desconocida";
            }

            // Redimensionar el buffer para particiones no soportadas
            UnsupportedPartitions = ReallocatePool(
                UnsupportedCount * sizeof(UnsupportedPartitionInfo),
                (UnsupportedCount + 1) * sizeof(UnsupportedPartitionInfo),
                UnsupportedPartitions
            );

            if (UnsupportedPartitions == NULL) {
                return EFI_OUT_OF_RESOURCES;
            }

            UnsupportedPartitions[UnsupportedCount].DevicePathStr = DevicePathStr;
            UnsupportedPartitions[UnsupportedCount].PartitionSize = PartitionSize;
            UnsupportedCount++;
        }
    }

    // Liberar el buffer del manejador
    FreePool(HandleBuffer);

    // Imprimir particiones soportadas
    Print(L"\nParticiones que soportan el sistema de archivos simple:\n");
    for (i = 0; i < SupportedCount; i++) {
        Print(L"-%s |%llu bytes\n", SupportedPartitions[i].DevicePathStr, SupportedPartitions[i].PartitionSize);
		//Print(L"Contenido del directorio:\n");
        for (UINTN j = 0; j < SupportedPartitions[i].FileCount; j++) {
			//Print(L"  %s\n", SupportedPartitions[i].Files[j]);
            FreePool(SupportedPartitions[i].Files[j]);
        }
        FreePool(SupportedPartitions[i].Files);
        FreePool(SupportedPartitions[i].DevicePathStr);
    }
    FreePool(SupportedPartitions);

    Status = WriteDataToUnsupportedPartition(BlockIo, 0, 4096, 0x41); 
     if (EFI_ERROR(Status)) {
         return Status;
    }
    // Imprimir particiones no soportadas
    Print(L"\nParticiones que no soportan el sistema de archivos simple:\n");
    for (i = 0; i < UnsupportedCount; i++) {
        Print(L"-%s |%llu bytes\n", UnsupportedPartitions[i].DevicePathStr, UnsupportedPartitions[i].PartitionSize);
        
        // Leer y mostrar los primeros 4096 bytes de la partición no soportada y volcar a un archivo
        CHAR16 FilePath[256];
        UnicodeSPrint(FilePath, sizeof(FilePath), L"\\EFI\\Boot\\unsupported_partition_data_%u.bin", i);
        Status = ReadRawDataWrite(BlockIo, 0, 4096, FilePath);
        if (EFI_ERROR(Status)) {
            Print(L"Error al leer datos de la partición no soportada: %r\n", Status);
        }
        
        FreePool(UnsupportedPartitions[i].DevicePathStr);
    }
    FreePool(UnsupportedPartitions);

    return EFI_SUCCESS;
}