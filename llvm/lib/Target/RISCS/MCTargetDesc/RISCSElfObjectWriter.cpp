//===-- RISCSELFObjectWriter.cpp - RISCS ELF Writer -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/RISCSFixupKinds.h"
#include "MCTargetDesc/RISCSMCExpr.h"
#include "MCTargetDesc/RISCSMCTargetDesc.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class RISCSELFObjectWriter : public MCELFObjectTargetWriter {
public:
  RISCSELFObjectWriter(uint8_t OSABI, bool Is64Bit);

  ~RISCSELFObjectWriter() override;

  // Return true if the given relocation must be with a symbol rather than
  // section plus offset.
  bool needsRelocateWithSymbol(const MCValue &Val, const MCSymbol &Sym,
                               unsigned Type) const override {
    return true;
  }

protected:
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
};
}

RISCSELFObjectWriter::RISCSELFObjectWriter(uint8_t OSABI, bool Is64Bit)
    : MCELFObjectTargetWriter(Is64Bit, OSABI, ELF::EM_RISCS,
                              /*HasRelocationAddend*/ true) {}

RISCSELFObjectWriter::~RISCSELFObjectWriter() {}

unsigned RISCSELFObjectWriter::getRelocType(MCContext &Ctx,
                                             const MCValue &Target,
                                             const MCFixup &Fixup,
                                             bool IsPCRel) const {
  const MCExpr *Expr = Fixup.getValue();
  // Determine the type of the relocation
  unsigned Kind = Fixup.getTargetKind();
  if (Kind >= FirstLiteralRelocationKind)
    return Kind - FirstLiteralRelocationKind;
  if (IsPCRel) {
    switch (Kind) {
    default:
      Ctx.reportError(Fixup.getLoc(), "Unsupported relocation type");
      return ELF::R_RISCS_NONE;
    case FK_Data_4:
    case FK_PCRel_4:
      return ELF::R_RISCS_32_PCREL;
    case riscs::fixup_RISCS_pcrel_hi20:
      return ELF::R_RISCS_PCREL_HI20;
    case riscs::fixup_RISCS_pcrel_lo12_i:
      return ELF::R_RISCS_PCREL_LO12_I;
    case riscs::fixup_RISCS_pcrel_lo12_s:
      return ELF::R_RISCS_PCREL_LO12_S;
    case riscs::fixup_RISCS_got_hi20:
      return ELF::R_RISCS_GOT_HI20;
    case riscs::fixup_RISCS_tls_got_hi20:
      return ELF::R_RISCS_TLS_GOT_HI20;
    case riscs::fixup_RISCS_tls_gd_hi20:
      return ELF::R_RISCS_TLS_GD_HI20;
    case riscs::fixup_RISCS_jal:
      return ELF::R_RISCS_JAL;
    case riscs::fixup_RISCS_branch:
      return ELF::R_RISCS_BRANCH;
    case riscs::fixup_RISCS_call:
      return ELF::R_RISCS_CALL;
    case riscs::fixup_RISCS_call_plt:
      return ELF::R_RISCS_CALL_PLT;
    case riscs::fixup_RISCS_add_8:
      return ELF::R_RISCS_ADD8;
    case riscs::fixup_RISCS_sub_8:
      return ELF::R_RISCS_SUB8;
    case riscs::fixup_RISCS_add_16:
      return ELF::R_RISCS_ADD16;
    case riscs::fixup_RISCS_sub_16:
      return ELF::R_RISCS_SUB16;
    case riscs::fixup_RISCS_add_32:
      return ELF::R_RISCS_ADD32;
    case riscs::fixup_RISCS_sub_32:
      return ELF::R_RISCS_SUB32;
    case riscs::fixup_RISCS_add_64:
      return ELF::R_RISCS_ADD64;
    case riscs::fixup_RISCS_sub_64:
      return ELF::R_RISCS_SUB64;
    }
  }

  switch (Kind) {
  default:
    Ctx.reportError(Fixup.getLoc(), "Unsupported relocation type");
    return ELF::R_RISCS_NONE;
  case FK_Data_1:
    Ctx.reportError(Fixup.getLoc(), "1-byte data relocations not supported");
    return ELF::R_RISCS_NONE;
  case FK_Data_2:
    Ctx.reportError(Fixup.getLoc(), "2-byte data relocations not supported");
    return ELF::R_RISCS_NONE;
  case FK_Data_4:
    if (Expr->getKind() == MCExpr::Target &&
        cast<RISCSMCExpr>(Expr)->getKind() == RISCSMCExpr::VK_RISCS_32_PCREL)
      return ELF::R_RISCS_32_PCREL;
    return ELF::R_RISCS_32;
  case FK_Data_8:
    return ELF::R_RISCS_64;
  case riscs::fixup_RISCS_hi20:
    return ELF::R_RISCS_HI20;
  case riscs::fixup_RISCS_lo12_i:
    return ELF::R_RISCS_LO12_I;
  case riscs::fixup_RISCS_lo12_s:
    return ELF::R_RISCS_LO12_S;
  case riscs::fixup_RISCS_tprel_hi20:
    return ELF::R_RISCS_TPREL_HI20;
  case riscs::fixup_RISCS_tprel_lo12_i:
    return ELF::R_RISCS_TPREL_LO12_I;
  case riscs::fixup_RISCS_tprel_lo12_s:
    return ELF::R_RISCS_TPREL_LO12_S;
  case riscs::fixup_RISCS_tprel_add:
    return ELF::R_RISCS_TPREL_ADD;
  case riscs::fixup_RISCS_relax:
    return ELF::R_RISCS_RELAX;
  case riscs::fixup_RISCS_align:
    return ELF::R_RISCS_ALIGN;
  case riscs::fixup_RISCS_set_6b:
    return ELF::R_RISCS_SET6;
  case riscs::fixup_RISCS_sub_6b:
    return ELF::R_RISCS_SUB6;
  case riscs::fixup_RISCS_add_8:
    return ELF::R_RISCS_ADD8;
  case riscs::fixup_RISCS_set_8:
    return ELF::R_RISCS_SET8;
  case riscs::fixup_RISCS_sub_8:
    return ELF::R_RISCS_SUB8;
  case riscs::fixup_RISCS_set_16:
    return ELF::R_RISCS_SET16;
  case riscs::fixup_RISCS_add_16:
    return ELF::R_RISCS_ADD16;
  case riscs::fixup_RISCS_sub_16:
    return ELF::R_RISCS_SUB16;
  case riscs::fixup_RISCS_set_32:
    return ELF::R_RISCS_SET32;
  case riscs::fixup_RISCS_add_32:
    return ELF::R_RISCS_ADD32;
  case riscs::fixup_RISCS_sub_32:
    return ELF::R_RISCS_SUB32;
  case riscs::fixup_RISCS_add_64:
    return ELF::R_RISCS_ADD64;
  case riscs::fixup_RISCS_sub_64:
    return ELF::R_RISCS_SUB64;
  }
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createRISCSELFObjectWriter(uint8_t OSABI, bool Is64Bit) {
  return std::make_unique<RISCSELFObjectWriter>(OSABI, Is64Bit);
}
