add_llvm_component_library(LLVMRISCSDesc
  RISCSMCTargetDesc.cpp

  LINK_COMPONENTS
  RISCSInfo
  MC
  Support

  ADD_TO_COMPONENT
  RISCS
  )
