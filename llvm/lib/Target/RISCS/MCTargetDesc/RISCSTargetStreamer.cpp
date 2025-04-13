//===-- RISCSTargetStreamer.cpp - riscs Target Streamer Methods -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides riscs specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "RISCSTargetStreamer.h"
#include "RISCSInfo.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/RISCSAttributes.h"
#include "llvm/Support/RISCSISAInfo.h"

using namespace llvm;

RISCSTargetStreamer::RISCSTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

void RISCSTargetStreamer::finish() { finishAttributeSection(); }

void RISCSTargetStreamer::emitDirectiveOptionPush() {}
void RISCSTargetStreamer::emitDirectiveOptionPop() {}
void RISCSTargetStreamer::emitDirectiveOptionPIC() {}
void RISCSTargetStreamer::emitDirectiveOptionNoPIC() {}
void RISCSTargetStreamer::emitDirectiveOptionRelax() {}
void RISCSTargetStreamer::emitDirectiveOptionNoRelax() {}
void RISCSTargetStreamer::emitAttribute(unsigned Attribute, unsigned Value) {}
void RISCSTargetStreamer::finishAttributeSection() {}
void RISCSTargetStreamer::emitTextAttribute(unsigned Attribute,
                                            StringRef String) {}
void RISCSTargetStreamer::emitIntTextAttribute(unsigned Attribute,
                                               unsigned IntValue,
                                               StringRef StringValue) {}

void RISCSTargetStreamer::emitTargetAttributes(const MCSubtargetInfo &STI) {
  emitAttribute(RISCSAttrs::STACK_ALIGN, RISCSAttrs::ALIGN_16);

  unsigned XLen = 64;
  std::vector<std::string> FeatureVector;
  riscsFeatures::toFeatureVector(FeatureVector, STI.getFeatureBits());

  auto ParseResult = llvm::RISCSISAInfo::parseFeatures(XLen, FeatureVector);
  if (!ParseResult) {
    /* Assume any error about features should handled earlier.  */
    consumeError(ParseResult.takeError());
    llvm_unreachable("Parsing feature error when emitTargetAttributes?");
  } else {
    auto &ISAInfo = *ParseResult;
    emitTextAttribute(RISCSAttrs::ARCH, ISAInfo->toString());
  }
}

// This part is for ascii assembly output
RISCSTargetAsmStreamer::RISCSTargetAsmStreamer(MCStreamer &S,
                                               formatted_raw_ostream &OS)
    : RISCSTargetStreamer(S), OS(OS) {}

void RISCSTargetAsmStreamer::emitDirectiveOptionPush() {
  OS << "\t.option\tpush\n";
}

void RISCSTargetAsmStreamer::emitDirectiveOptionPop() {
  OS << "\t.option\tpop\n";
}

void RISCSTargetAsmStreamer::emitDirectiveOptionPIC() {
  OS << "\t.option\tpic\n";
}

void RISCSTargetAsmStreamer::emitDirectiveOptionNoPIC() {
  OS << "\t.option\tnopic\n";
}

void RISCSTargetAsmStreamer::emitDirectiveOptionRelax() {
  OS << "\t.option\trelax\n";
}

void RISCSTargetAsmStreamer::emitDirectiveOptionNoRelax() {
  OS << "\t.option\tnorelax\n";
}

void RISCSTargetAsmStreamer::emitAttribute(unsigned Attribute, unsigned Value) {
  OS << "\t.attribute\t" << Attribute << ", " << Twine(Value) << "\n";
}

void RISCSTargetAsmStreamer::emitTextAttribute(unsigned Attribute,
                                               StringRef String) {
  OS << "\t.attribute\t" << Attribute << ", \"" << String << "\"\n";
}

void RISCSTargetAsmStreamer::emitIntTextAttribute(unsigned Attribute,
                                                  unsigned IntValue,
                                                  StringRef StringValue) {}

void RISCSTargetAsmStreamer::finishAttributeSection() {}
