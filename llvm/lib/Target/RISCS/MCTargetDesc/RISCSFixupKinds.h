#ifndef __LLVM_LIB_TARGET_RISCS_MCTARGETDESC_RISCSFIXUPKINDS_H__
#define __LLVM_LIB_TARGET_RISCS_MCTARGETDESC_RISCSFIXUPKINDS_H__

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace riscs {
enum Fixups {
  // 20-bit fixup corresponding to %hi(foo) for instructions like lui
  fixup_RISCS_hi20 = FirstTargetFixupKind,
  // 12-bit fixup corresponding to %lo(foo) for instructions like addi
  fixup_RISCS_lo12_i,
  // 12-bit fixup corresponding to %lo(foo) for the S-type store instructions
  fixup_RISCS_lo12_s,
  // 20-bit fixup corresponding to %pcrel_hi(foo) for instructions like auipc
  fixup_RISCS_pcrel_hi20,
  // 12-bit fixup corresponding to %pcrel_lo(foo) for instructions like addi
  fixup_RISCS_pcrel_lo12_i,
  // 12-bit fixup corresponding to %pcrel_lo(foo) for the S-type store
  // instructions
  fixup_RISCS_pcrel_lo12_s,
  // 20-bit fixup corresponding to %got_pcrel_hi(foo) for instructions like
  // auipc
  fixup_RISCS_got_hi20,
  // 20-bit fixup corresponding to %tprel_hi(foo) for instructions like lui
  fixup_RISCS_tprel_hi20,
  // 12-bit fixup corresponding to %tprel_lo(foo) for instructions like addi
  fixup_RISCS_tprel_lo12_i,
  // 12-bit fixup corresponding to %tprel_lo(foo) for the S-type store
  // instructions
  fixup_RISCS_tprel_lo12_s,
  // Fixup corresponding to %tprel_add(foo) for PseudoAddTPRel, used as a linker
  // hint
  fixup_RISCS_tprel_add,
  // 20-bit fixup corresponding to %tls_ie_pcrel_hi(foo) for instructions like
  // auipc
  fixup_RISCS_tls_got_hi20,
  // 20-bit fixup corresponding to %tls_gd_pcrel_hi(foo) for instructions like
  // auipc
  fixup_RISCS_tls_gd_hi20,
  // 20-bit fixup for symbol references in the jal instruction
  fixup_RISCS_jal,
  // 12-bit fixup for symbol references in the branch instructions
  fixup_RISCS_branch,
  // Fixup representing a legacy no-pic function call attached to the auipc
  // instruction in a pair composed of adjacent auipc+jalr instructions.
  fixup_RISCS_call,
  // Fixup representing a function call attached to the auipc instruction in a
  // pair composed of adjacent auipc+jalr instructions.
  fixup_RISCS_call_plt,
  // Used to generate an R_RISCS_RELAX relocation, which indicates the linker
  // may relax the instruction pair.
  fixup_RISCS_relax,
  // Used to generate an R_RISCS_ALIGN relocation, which indicates the linker
  // should fixup the alignment after linker relaxation.
  fixup_RISCS_align,
  // 8-bit fixup corresponding to R_RISCS_SET8 for local label assignment.
  fixup_RISCS_set_8,
  // 8-bit fixup corresponding to R_RISCS_ADD8 for 8-bit symbolic difference
  // paired relocations.
  fixup_RISCS_add_8,
  // 8-bit fixup corresponding to R_RISCS_SUB8 for 8-bit symbolic difference
  // paired relocations.
  fixup_RISCS_sub_8,
  // 16-bit fixup corresponding to R_RISCS_SET16 for local label assignment.
  fixup_RISCS_set_16,
  // 16-bit fixup corresponding to R_RISCS_ADD16 for 16-bit symbolic difference
  // paired reloctions.
  fixup_RISCS_add_16,
  // 16-bit fixup corresponding to R_RISCS_SUB16 for 16-bit symbolic difference
  // paired reloctions.
  fixup_RISCS_sub_16,
  // 32-bit fixup corresponding to R_RISCS_SET32 for local label assignment.
  fixup_RISCS_set_32,
  // 32-bit fixup corresponding to R_RISCS_ADD32 for 32-bit symbolic difference
  // paired relocations.
  fixup_RISCS_add_32,
  // 32-bit fixup corresponding to R_RISCS_SUB32 for 32-bit symbolic difference
  // paired relocations.
  fixup_RISCS_sub_32,
  // 64-bit fixup corresponding to R_RISCS_ADD64 for 64-bit symbolic difference
  // paired relocations.
  fixup_RISCS_add_64,
  // 64-bit fixup corresponding to R_RISCS_SUB64 for 64-bit symbolic difference
  // paired relocations.
  fixup_RISCS_sub_64,
  // 6-bit fixup corresponding to R_RISCS_SET6 for local label assignment in
  // DWARF CFA.
  fixup_RISCS_set_6b,
  // 6-bit fixup corresponding to R_RISCS_SUB6 for local label assignment in
  // DWARF CFA.
  fixup_RISCS_sub_6b,

  // Used as a sentinel, must be the last
  fixup_RISCS_invalid,
  NumTargetFixupKinds = fixup_RISCS_invalid - FirstTargetFixupKind
};
} // namespace riscs
} // end namespace llvm

#endif // __LLVM_LIB_TARGET_RISCS_MCTARGETDESC_RISCSFIXUPKINDS_H__
