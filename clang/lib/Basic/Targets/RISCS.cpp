
#include "RISCS.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/MacroBuilder.h"
#include "clang/Basic/TargetBuiltins.h"

using namespace clang;
using namespace clang::targets;

static constexpr llvm::StringTable BuiltinStrings =
    CLANG_BUILTIN_STR_TABLE_START
#define BUILTIN CLANG_BUILTIN_STR_TABLE
#define TARGET_BUILTIN CLANG_TARGET_BUILTIN_STR_TABLE
#define TARGET_HEADER_BUILTIN CLANG_TARGET_HEADER_BUILTIN_STR_TABLE
#include "clang/Basic/BuiltinsRISCS.def"
;

static constexpr int NumRISCSBuiltins = clang::riscs::LastTSBuiltin - Builtin::FirstTSBuiltin;

static constexpr auto BuiltinInfos = Builtin::MakeInfos<NumRISCSBuiltins>({
#define BUILTIN CLANG_BUILTIN_ENTRY
#define LANGBUILTIN CLANG_LANGBUILTIN_ENTRY
#define LIBBUILTIN CLANG_LIBBUILTIN_ENTRY
#define TARGET_BUILTIN CLANG_TARGET_BUILTIN_ENTRY
#define TARGET_HEADER_BUILTIN CLANG_TARGET_HEADER_BUILTIN_ENTRY
#include "clang/Basic/BuiltinsRISCS.def"
});

// static constexpr Builtin::Info BuiltinInfos[] = {
// #define BUILTIN(ID, TYPE, ATTRS) \
//   {#ID, TYPE, ATTRS, nullptr, HeaderDesc::NO_HEADER, ALL_LANGUAGES},
// #include "clang/Basic/BuiltinsRISCS.def"
// };

void RISCSTargetInfo::getTargetDefines(const LangOptions &Opts,
                                        MacroBuilder &Builder) const {
  Builder.defineMacro("__riscs__");
}

llvm::SmallVector<Builtin::InfosShard>
RISCSTargetInfo::getTargetBuiltins() const {
  return {{&BuiltinStrings, BuiltinInfos}};
}
