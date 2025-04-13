add_llvm_component_group(RISCS)

set(LLVM_TARGET_DEFINITIONS RISCS.td)

tablegen(LLVM RISCSGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM RISCSGenInstrsInfo.inc -gen-instr-info)
tablegen(LLVM RISCSGenCallingConv.inc -gen-callingconv)
tablegen(LLVM RISCSGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM RISCSGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM RISCSGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM RISCSGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM RISCSGenMCCodeEmitter.inc -gen-emitter)

add_public_tablegen_target(RISCSCommonTableGen)

add_llvm_target(RISCSCodeGen
  RISCSTargetMachine.cpp
  RISCSSubtarget.cpp

  RISCSRegisterInfo.cpp
  RISCSInstrsInfo.cpp

  RISCSISelLowering.cpp
  RISCSFrameLowering.cpp

  RISCSISelDagToDag.cpp
  RISCSAsmPrinter.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  MC
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
