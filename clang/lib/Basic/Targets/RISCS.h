#ifndef __CLANG_LIB_BASIC_TARGETS_RISCS_H__
#define __CLANG_LIB_BASIC_TARGETS_RISCS_H__

#include <array>

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/Support/Compiler.h"

namespace clang::targets {

class LLVM_LIBRARY_VISIBILITY RISCSTargetInfo : public TargetInfo {
public:
  RISCSTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {

    LongWidth = LongAlign = PointerWidth = PointerAlign = 64;
    IntMaxType = Int64Type = SignedLong;

    WCharType = SignedInt;
    WIntType = UnsignedInt;

    NoAsmVariants = true;
    LongLongAlign = 64;
    SuitableAlign = 64;
    SizeType = UnsignedLong;
    PtrDiffType = SignedLong;
    IntPtrType = SignedLong;
    UseZeroLengthBitfieldAlignment = true;
    resetDataLayout("e-m:e-p:64:64-i64:64-i128:128-n32:64-S128");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  llvm::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  std::string_view getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override {
    static constexpr const std::array<const char*, 32> GCCRegNames = {
      // Integer registers
      "x0",  "x1",  "x2",  "x3",  "x4",  "x5",  "x6",  "x7",
      "x8",  "x9",  "x10", "x11", "x12", "x13", "x14", "x15",
      "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
      "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31",
    };
    return GCCRegNames;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return std::nullopt;
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    return false;
  }

  bool hasBitIntType() const override { return true; }

  bool isCLZForZeroUndef() const override { return false; }
};

} // namespace clang::targets

#endif // __CLANG_LIB_BASIC_TARGETS_RISCS_H__
