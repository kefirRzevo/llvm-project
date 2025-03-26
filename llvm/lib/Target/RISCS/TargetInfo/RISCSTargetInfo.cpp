//===-- RISCSTargetInfo.cpp - RISC-S Target Implementation ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/RISCSTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheRISCSTarget() {
  static Target TheRISCSTarget;
  return TheRISCSTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRISCSTargetInfo() {
  RegisterTarget<Triple::riscs, /*HasJIT=*/false> X(
        getTheRISCSTarget(), "riscs", "64-bit RISC-S", "RISCS");
}
