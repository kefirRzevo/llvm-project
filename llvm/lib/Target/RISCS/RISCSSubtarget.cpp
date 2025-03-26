#include "RISCSSubtarget.h"

using namespace llvm;

#define DEBUG_TYPE "riscs-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "RISCSGenSubtargetInfo.inc"

void RISCSSubtarget::anchor() {}

RISCSSubtarget::RISCSSubtarget(const Triple &TT, const std::string &CPU,
                             const std::string &FS, const TargetMachine &TM)
    : RISCSGenSubtargetInfo(TT, CPU, /*TuneCPU=*/CPU, FS), InstrInfo(*this),
      FrameLowering(*this), TLInfo(TM, *this) {}
