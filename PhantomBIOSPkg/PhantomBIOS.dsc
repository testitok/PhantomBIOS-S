[Defines]
  DSC_SPECIFICATION         = 0x0001001A
  PLATFORM_GUID             = 6bddbee1-fceb-4e26-9673-c904912bab01
  PLATFORM_VERSION          = 0.01
  PLATFORM_NAME             = PhantomBIOS
  SKUID_IDENTIFIER          = DEFAULT
  SUPPORTED_ARCHITECTURES   = X64
  BUILD_TARGETS             = DEBUG|RELEASE|NOOPT
  OUTPUT_DIRECTORY          = Build/PhantomBIOS

[LibraryClasses]
  #
  # Basic
  #
  RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf

  #
  # UEFI & PI
  #
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf

  #
  # Misc
  #
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

#[LibraryClasses.ARM,LibraryClasses.AARCH64]
  #
  # It is not possible to prevent the ARM compiler for generic intrinsic functions.
  # This library provides the instrinsic functions generate by a given compiler.
  # [LibraryClasses.ARM] and NULL mean link this library into all ARM images.
  #
  #NULL|ArmPkg/Library/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf

  # Add support for GCC stack protector
  #NULL|MdePkg/Library/BaseStackCheckLib/BaseStackCheckLib.inf

[Components]
  PhantomBIOSPkg/PhantomBIOS.inf
