//===-- RISCSMCCodeEmitter.cpp - Convert RISCS code to machine code -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "RISCSFixupKinds.h"
#include "RISCSInfo.h"
#include "RISCSMCExpr.h"
#include "RISCSMCTargetDesc.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/raw_ostream.h"

#define GET_REGINFO_ENUM
#include "RISCSGenRegisterInfo.inc"

#define GET_COMPUTE_FEATURES
#define GET_INSTRINFO_MC_HELPER_DECLS
#define GET_INSTRINFO_ENUM
#include "RISCSGenInstrsInfo.inc"

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");
STATISTIC(MCNumFixups, "Number of MC fixups created");

namespace {
class RISCSMCCodeEmitter : public MCCodeEmitter {
  RISCSMCCodeEmitter(const RISCSMCCodeEmitter &) = delete;
  void operator=(const RISCSMCCodeEmitter &) = delete;
  MCContext &Ctx;
  MCInstrInfo const &MCII;

public:
  RISCSMCCodeEmitter(MCContext &ctx, MCInstrInfo const &MCII)
      : Ctx(ctx), MCII(MCII) {}

  ~RISCSMCCodeEmitter() override {}

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  void expandFunctionCall(const MCInst &MI, SmallVectorImpl<char> &CB,
                          SmallVectorImpl<MCFixup> &Fixups,
                          const MCSubtargetInfo &STI) const;

  // void expandAddTPRel(const MCInst &MI, raw_ostream &OS,
  //                     SmallVectorImpl<MCFixup> &Fixups,
  //                     const MCSubtargetInfo &STI) const;

  /// TableGen'erated function for getting the binary encoding for an
  /// instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// Return binary encoding of operand. If the machine operand requires
  /// relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getImmOpValueAsr1(const MCInst &MI, unsigned OpNo,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getImmOpValue(const MCInst &MI, unsigned OpNo,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const;
};
} // end anonymous namespace

MCCodeEmitter *llvm::createRISCSMCCodeEmitter(const MCInstrInfo &MCII,
                                              MCContext &Ctx) {
  return new RISCSMCCodeEmitter(Ctx, MCII);
}

void RISCSMCCodeEmitter::expandFunctionCall(const MCInst &MI,
                                            SmallVectorImpl<char> &CB,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  MCInst TmpInst;
  MCOperand Func;
  MCRegister Ra;
  uint32_t Binary;

  if (MI.getOpcode() == riscs::PseudoCALL) {
    Func = MI.getOperand(0);
    Ra = riscs::X1;
  }
  assert(Func.isExpr() && "Expected expression");

  const MCExpr *CallExpr = Func.getExpr();

  // Emit AUIPC Ra, Func with R_RISCS_CALL relocation type.
  TmpInst = MCInstBuilder(riscs::AUIPC)
                .addReg(Ra)
                .addOperand(MCOperand::createExpr(CallExpr));
  Binary = getBinaryCodeForInstr(TmpInst, Fixups, STI);
  support::endian::write(CB, Binary, endianness::little);

  // Emit JALR Ra, Ra, 0
  TmpInst = MCInstBuilder(riscs::JALR).addReg(Ra).addReg(Ra).addImm(0);
  Binary = getBinaryCodeForInstr(TmpInst, Fixups, STI);
  support::endian::write(CB, Binary, endianness::little);
}

// Expand PseudoAddTPRel to a simple ADD with the correct relocation.
// void RISCSMCCodeEmitter::expandAddTPRel(const MCInst &MI, raw_ostream &OS,
//                                          SmallVectorImpl<MCFixup> &Fixups,
//                                          const MCSubtargetInfo &STI) const {
//   MCOperand DestReg = MI.getOperand(0);
//   MCOperand SrcReg = MI.getOperand(1);
//   MCOperand TPReg = MI.getOperand(2);
//   assert(TPReg.isReg() && TPReg.getReg() == riscs::X4 &&
//          "Expected thread pointer as second input to TP-relative add");

//   MCOperand SrcSymbol = MI.getOperand(3);
//   assert(SrcSymbol.isExpr() &&
//          "Expected expression as third input to TP-relative add");

//   const RISCSMCExpr *Expr = dyn_cast<RISCSMCExpr>(SrcSymbol.getExpr());
//   assert(Expr && Expr->getKind() == RISCSMCExpr::VK_RISCS_TPREL_ADD &&
//          "Expected tprel_add relocation on TP-relative symbol");

//   // Emit the correct tprel_add relocation for the symbol.
//   Fixups.push_back(MCFixup::create(
//       0, Expr, MCFixupKind(riscs::fixup_RISCS_tprel_add), MI.getLoc()));

//   // Emit a normal ADD instruction with the given operands.
//   MCInst TmpInst = MCInstBuilder(riscs::ADD)
//                        .addOperand(DestReg)
//                        .addOperand(SrcReg)
//                        .addOperand(TPReg);
//   uint32_t Binary = getBinaryCodeForInstr(TmpInst, Fixups, STI);
//   support::endian::write(OS, Binary, endianness::little);
// }

void RISCSMCCodeEmitter::encodeInstruction(const MCInst &MI,
                                           SmallVectorImpl<char> &CB,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  RISCS_MC::verifyInstructionPredicates(
      MI.getOpcode(), RISCS_MC::computeAvailableFeatures(STI.getFeatureBits()));

  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  // Get byte count of instruction.
  unsigned Size = Desc.getSize();

  std::string msg;
  raw_string_ostream Msg(msg);
  Msg << MI;
  printf("[DUMP] %s\n\n", Msg.str().c_str());
  // RISCSInstrInfo::getInstSizeInBytes expects that the total size of the
  // expanded instructions for each pseudo is correct in the Size field of the
  // tablegen definition for the pseudo.
  if (MI.getOpcode() == riscs::PseudoCALL) {
    expandFunctionCall(MI, CB, Fixups, STI);
    MCNumEmitted += 2;
    return;
  }

  switch (Size) {
  default:
    llvm_unreachable("Unhandled encodeInstruction length!");
  // case 2: {
  //   uint16_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
  //   support::endian::write<uint16_t>(CB, Bits, endianness::little);
  //   break;
  // }
  case 4: {
    uint32_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
    support::endian::write(CB, Bits, endianness::little);
    break;
  }
  }

  ++MCNumEmitted; // Keep track of the # of mi's emitted.
}

unsigned
RISCSMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {

  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  llvm_unreachable("Unhandled expression!");
  return 0;
}

unsigned
RISCSMCCodeEmitter::getImmOpValueAsr1(const MCInst &MI, unsigned OpNo,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);

  if (MO.isImm()) {
    unsigned Res = MO.getImm();
    assert((Res & 1) == 0 && "LSB is non-zero");
    return Res >> 1;
  }

  return getImmOpValue(MI, OpNo, Fixups, STI);
}

unsigned RISCSMCCodeEmitter::getImmOpValue(const MCInst &MI, unsigned OpNo,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);

  MCInstrDesc const &Desc = MCII.get(MI.getOpcode());
  unsigned MIFrm = riscsII::getFormat(Desc.TSFlags);

  // If the destination is an immediate, there is nothing to do.
  if (MO.isImm())
    return MO.getImm();

  assert(MO.isExpr() && "getImmOpValue expects only expressions or immediates");
  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();
  riscs::Fixups FixupKind = riscs::fixup_RISCS_invalid;
  if (Kind == MCExpr::Target) {
    const RISCSMCExpr *RVExpr = cast<RISCSMCExpr>(Expr);

    switch (RVExpr->getKind()) {
    case RISCSMCExpr::VK_RISCS_None:
    case RISCSMCExpr::VK_RISCS_Invalid:
    case RISCSMCExpr::VK_RISCS_32_PCREL:
      llvm_unreachable("Unhandled fixup kind!");
    case RISCSMCExpr::VK_RISCS_TPREL_ADD:
      // tprel_add is only used to indicate that a relocation should be emitted
      // for an add instruction used in TP-relative addressing. It should not be
      // expanded as if representing an actual instruction operand and so to
      // encounter it here is an error.
      llvm_unreachable(
          "VK_RISCS_TPREL_ADD should not represent an instruction operand");
    case RISCSMCExpr::VK_RISCS_LO:
      if (MIFrm == riscsII::InstFormatI)
        FixupKind = riscs::fixup_RISCS_lo12_i;
      else if (MIFrm == riscsII::InstFormatS)
        FixupKind = riscs::fixup_RISCS_lo12_s;
      else
        llvm_unreachable("VK_RISCS_LO used with unexpected instruction format");
      break;
    case RISCSMCExpr::VK_RISCS_HI:
      FixupKind = riscs::fixup_RISCS_hi20;
      break;
    case RISCSMCExpr::VK_RISCS_PCREL_LO:
      if (MIFrm == riscsII::InstFormatI)
        FixupKind = riscs::fixup_RISCS_pcrel_lo12_i;
      else if (MIFrm == riscsII::InstFormatS)
        FixupKind = riscs::fixup_RISCS_pcrel_lo12_s;
      else
        llvm_unreachable(
            "VK_RISCS_PCREL_LO used with unexpected instruction format");
      break;
    case RISCSMCExpr::VK_RISCS_PCREL_HI:
      FixupKind = riscs::fixup_RISCS_pcrel_hi20;
      break;
    case RISCSMCExpr::VK_RISCS_GOT_HI:
      FixupKind = riscs::fixup_RISCS_got_hi20;
      break;
    case RISCSMCExpr::VK_RISCS_TPREL_LO:
      if (MIFrm == riscsII::InstFormatI)
        FixupKind = riscs::fixup_RISCS_tprel_lo12_i;
      else if (MIFrm == riscsII::InstFormatS)
        FixupKind = riscs::fixup_RISCS_tprel_lo12_s;
      else
        llvm_unreachable(
            "VK_RISCS_TPREL_LO used with unexpected instruction format");
      break;
    case RISCSMCExpr::VK_RISCS_TPREL_HI:
      FixupKind = riscs::fixup_RISCS_tprel_hi20;
      break;
    case RISCSMCExpr::VK_RISCS_TLS_GOT_HI:
      FixupKind = riscs::fixup_RISCS_tls_got_hi20;
      break;
    case RISCSMCExpr::VK_RISCS_TLS_GD_HI:
      FixupKind = riscs::fixup_RISCS_tls_gd_hi20;
      break;
    case RISCSMCExpr::VK_RISCS_CALL:
      FixupKind = riscs::fixup_RISCS_call;
      break;
    case RISCSMCExpr::VK_RISCS_CALL_PLT:
      FixupKind = riscs::fixup_RISCS_call_plt;
      break;
    }
  } else if (Kind == MCExpr::SymbolRef &&
             cast<MCSymbolRefExpr>(Expr)->getKind() ==
                 MCSymbolRefExpr::VK_None) {
    if (MIFrm == riscsII::InstFormatJ) {
      FixupKind = riscs::fixup_RISCS_jal;
    } else if (MIFrm == riscsII::InstFormatB) {
      FixupKind = riscs::fixup_RISCS_branch;
    } else if (MIFrm == riscsII::InstFormatU) {
      FixupKind = riscs::fixup_RISCS_call;
    } else {
      llvm_unreachable("Unhandled fixup");
    }
  }

  assert(FixupKind != riscs::fixup_RISCS_invalid && "Unhandled expression!");

  Fixups.push_back(
      MCFixup::create(0, Expr, MCFixupKind(FixupKind), MI.getLoc()));
  ++MCNumFixups;

  return 0;
}

#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "RISCSGenMCCodeEmitter.inc"
