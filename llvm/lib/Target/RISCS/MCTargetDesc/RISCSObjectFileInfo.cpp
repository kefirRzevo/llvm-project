//===-- RISCSMCObjectFileInfo.cpp - riscs object file properties ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the RISCSMCObjectFileInfo properties.
//
//===----------------------------------------------------------------------===//

#include "RISCSObjectFileInfo.h"

using namespace llvm;

unsigned RISCSMCObjectFileInfo::getTextSectionAlignment() const {
  return 4;
}
