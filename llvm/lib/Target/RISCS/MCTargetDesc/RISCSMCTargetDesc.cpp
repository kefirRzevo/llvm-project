//===-- RISCSMCTargetDesc.cpp - RISCS Target Descriptions
//-------------------===//
//
//                     The LLVM Compiler Infrastructure
//
//===----------------------------------------------------------------------===//
//
// This file provides RISC-S specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "RISCSMCTargetDesc.h"
#include "RISCSElfStreamer.h"
#include "RISCSInfo.h"
#include "RISCSInstPrinter.h"
#include "RISCSMCAsmInfo.h"
#include "RISCSObjectFileInfo.h"
#include "RISCSTargetStreamer.h"
#include "TargetInfo/RISCSTargetInfo.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define GET_REGINFO_ENUM
#define GET_REGINFO_MC_DESC
#include "RISCSGenRegisterInfo.inc"

#define ENABLE_INSTR_PREDICATE_VERIFIER
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_DESC
#include "RISCSGenInstrsInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "RISCSGenSubtargetInfo.inc"

static MCInstrInfo *createRISCSMCInstrInfo() {
  auto *X = new MCInstrInfo();
  InitRISCSMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createRISCSMCRegisterInfo(const Triple &TT) {
  auto *X = new MCRegisterInfo();
  InitRISCSMCRegisterInfo(X, riscs::X1);
  return X;
}

static MCSubtargetInfo *
createRISCSMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createRISCSMCSubtargetInfoImpl(TT, CPU, /*TuneCPU=*/CPU, FS);
}

static MCAsmInfo *createRISCSMCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT,
                                       const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new RISCSMCAsmInfo(TT);
  MCRegister SP = MRI.getDwarfRegNum(riscs::X2, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, SP, 0);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCInstPrinter *createRISCSMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  return new RISCSInstPrinter(MAI, MII, MRI);
}

static MCTargetStreamer *
createRISCSTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS,
                             MCInstPrinter *InstPrint) {
  return new RISCSTargetStreamer(S);
}

static MCObjectFileInfo *
createRISCSMCObjectFileInfo(MCContext &Ctx, bool PIC,
                            bool LargeCodeModel = false) {
  MCObjectFileInfo *MOFI = new RISCSMCObjectFileInfo();
  MOFI->initMCObjectFileInfo(Ctx, PIC, LargeCodeModel);
  return MOFI;
}

static MCTargetStreamer *
createRISCSObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  const Triple &TT = STI.getTargetTriple();
  if (TT.isOSBinFormatELF())
    return new RISCSTargetELFStreamer(S, STI);
  return nullptr;
}

class RISCSMCInstrAnalysis : public MCInstrAnalysis {
public:
  explicit RISCSMCInstrAnalysis(const MCInstrInfo *Info)
      : MCInstrAnalysis(Info) {}

  bool evaluateBranch(const MCInst &Inst, uint64_t Addr, uint64_t Size,
                      uint64_t &Target) const override {
    if (isConditionalBranch(Inst)) {
      int64_t Imm;
      if (Size == 2)
        Imm = Inst.getOperand(1).getImm();
      else
        Imm = Inst.getOperand(2).getImm();
      Target = Addr + Imm;
      return true;
    }

    if (Inst.getOpcode() == riscs::JAL) {
      Target = Addr + Inst.getOperand(1).getImm();
      return true;
    }

    return false;
  }
};

static MCInstrAnalysis *createRISCSInstrAnalysis(const MCInstrInfo *Info) {
  return new RISCSMCInstrAnalysis(Info);
}

static MCTargetStreamer *createRISCSNullTargetStreamer(MCStreamer &S) {
  return new RISCSTargetStreamer(S);
}

namespace {
MCStreamer *createRISCSELFStreamer(const Triple &T, MCContext &Context,
                                   std::unique_ptr<MCAsmBackend> &&MAB,
                                   std::unique_ptr<MCObjectWriter> &&MOW,
                                   std::unique_ptr<MCCodeEmitter> &&MCE) {
  return createRISCSELFStreamer(Context, std::move(MAB), std::move(MOW),
                                std::move(MCE));
}
} // end anonymous namespace

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRISCSTargetMC() {
  // Register the MC asm info.
  Target &TheRISCSTarget = getTheRISCSTarget();
  RegisterMCAsmInfoFn X(TheRISCSTarget, createRISCSMCAsmInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCObjectFileInfo(TheRISCSTarget,
                                           createRISCSMCObjectFileInfo);
  TargetRegistry::RegisterMCInstrInfo(TheRISCSTarget, createRISCSMCInstrInfo);
  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheRISCSTarget, createRISCSMCRegisterInfo);

  TargetRegistry::RegisterMCAsmBackend(TheRISCSTarget, createRISCSAsmBackend);
  TargetRegistry::RegisterMCCodeEmitter(TheRISCSTarget,
                                        createRISCSMCCodeEmitter);
  TargetRegistry::RegisterMCInstPrinter(TheRISCSTarget,
                                        createRISCSMCInstPrinter);
  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheRISCSTarget,
                                          createRISCSMCSubtargetInfo);
  TargetRegistry::RegisterELFStreamer(TheRISCSTarget, createRISCSELFStreamer);
  TargetRegistry::RegisterObjectTargetStreamer(TheRISCSTarget,
                                               createRISCSObjectTargetStreamer);
  TargetRegistry::RegisterMCInstrAnalysis(TheRISCSTarget,
                                          createRISCSInstrAnalysis);
  // Register the MCInstPrinter
  TargetRegistry::RegisterMCInstPrinter(TheRISCSTarget,
                                        createRISCSMCInstPrinter);

  TargetRegistry::RegisterAsmTargetStreamer(TheRISCSTarget,
                                            createRISCSTargetAsmStreamer);

  TargetRegistry::RegisterNullTargetStreamer(TheRISCSTarget,
                                             createRISCSNullTargetStreamer);
}
