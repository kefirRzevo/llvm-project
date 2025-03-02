
add_llvm_component_group(RISCS)

# Add tablegen in future here

add_llvm_target(RISCSCodeGen
  RISCSTargetMachine.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  GlobalISel
  RISCSInfo
  Scalar
  SelectionDAG
  Support
  Target
  TargetParser

  ADD_TO_COMPONENT
  RISCS
  )

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
