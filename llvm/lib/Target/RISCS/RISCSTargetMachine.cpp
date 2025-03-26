//===----------------------------------------------------------------------===//
//
// Implements the info about RISC-S target spec.
//
//===----------------------------------------------------------------------===//

#include "RISCSTargetMachine.h"
#include "RISCSMachineFunctionInfo.h"
#include "TargetInfo/RISCSTargetInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CodeGen.h"

#define DEBUG_TYPE "sim"

using namespace llvm;

static Reloc::Model getEffectiveRelocModel(const Triple &TT,
                                           std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

/// simTargetMachine ctor - Create an LP64 Architecture model
RISCSTargetMachine::RISCSTargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         std::optional<Reloc::Model> RM,
                                         std::optional<CodeModel::Model> CM,
                                         CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, "e-m:e-p:64:64-i64:64-i128:128-n32:64-S128",
                        TT, CPU, FS, Options, getEffectiveRelocModel(TT, RM),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  initAsmInfo();
}

RISCSTargetMachine::~RISCSTargetMachine() = default;

MachineFunctionInfo *RISCSTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return RISCSFunctionInfo::create<RISCSFunctionInfo>(Allocator, F, STI);
}

namespace {

class RISCSPassConfig : public TargetPassConfig {
public:
  RISCSPassConfig(RISCSTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  RISCSTargetMachine &getRISCSTargetMachine() const {
    return getTM<RISCSTargetMachine>();
  }

  bool addInstSelector() override;
};

} // anonymous namespace

TargetPassConfig *RISCSTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new RISCSPassConfig(*this, PM);
}

bool RISCSPassConfig::addInstSelector() {
  addPass(createRISCSISelDag(getRISCSTargetMachine(), getOptLevel()));
  return false;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRISCSTarget() {
  RegisterTargetMachine<RISCSTargetMachine> X(getTheRISCSTarget());
}
