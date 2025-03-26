#include "RISCSInfo.h"

#define GET_REGINFO_ENUM
#include "RISCSGenRegisterInfo.inc"

namespace llvm {
  namespace riscsABI {
    MCRegister getBPReg() { return riscs::X9; }
    MCRegister getSCSPReg() { return riscs::X18; }
  } // namespace riscsABI
} // namespace llvm
