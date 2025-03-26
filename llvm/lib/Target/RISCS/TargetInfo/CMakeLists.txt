add_llvm_component_library(LLVMRISCSInfo
  RISCSTargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  RISCS
  )
