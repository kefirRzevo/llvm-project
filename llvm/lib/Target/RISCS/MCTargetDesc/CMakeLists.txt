add_llvm_component_library(LLVMRISCSDesc
  RISCSMCTargetDesc.cpp
  RISCSInfo.cpp

  RISCSMCAsmInfo.cpp
  RISCSMCExpr.cpp
  RISCSObjectFileInfo.cpp
  RISCSInstPrinter.cpp
  RISCSTargetStreamer.cpp
  RISCSAsmBackend.cpp
  RISCSMCCodeEmitter.cpp
  RISCSElfObjectWriter.cpp
  RISCSElfStreamer.cpp

  LINK_COMPONENTS
  RISCSInfo
  MC
  Support

  ADD_TO_COMPONENT
  RISCS
  )
