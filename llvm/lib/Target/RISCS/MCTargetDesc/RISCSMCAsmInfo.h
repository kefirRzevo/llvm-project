#ifndef __LLVM_LIB_TARGET_RISCS_MCTARGETDESC_RISCSMCASMINFO_H__
#define __LLVM_LIB_TARGET_RISCS_MCTARGETDESC_RISCSMCASMINFO_H__

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {

class Triple;

class RISCSMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit RISCSMCAsmInfo(const Triple &TT);
};

} // end namespace llvm

#endif // __LLVM_LIB_TARGET_RISCS_MCTARGETDESC_RISCSMCASMINFO_H__
