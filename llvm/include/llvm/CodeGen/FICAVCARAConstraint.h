//===- llvm/CodeGen/FICAVCARAConstraint.h --------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the FICAVCABuilder interface, for classes which build FICAVCA
// instances to represent register allocation problems, and the RegAllocFICAVCA
// interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_FICAVCARACONSTRAINT_H
#define LLVM_CODEGEN_FICAVCARACONSTRAINT_H

#include <algorithm>
#include <memory>
#include <vector>

namespace llvm {

namespace FICAVCA {
namespace RegAlloc {

// Forward declare FICAVCA graph class.
class FICAVCARAGraph;

} // end namespace RegAlloc
} // end namespace FICAVCA

using FICAVCARAGraph = FICAVCA::RegAlloc::FICAVCARAGraph;

/// Abstract base for classes implementing FICAVCA register allocation
///        constraints (e.g. Spill-costs, interference, coalescing).
class FICAVCARAConstraint {
public:
  virtual ~FICAVCARAConstraint() = 0;
  virtual void apply(FICAVCARAGraph &G) = 0;

private:
  virtual void anchor();
};

/// FICAVCA register allocation constraint composer.
///
///   Constraints added to this list will be applied, in the order that they are
/// added, to the FICAVCA graph.
class FICAVCARAConstraintList : public FICAVCARAConstraint {
public:
  void apply(FICAVCARAGraph &G) override {
    for (auto &C : Constraints)
      C->apply(G);
  }

  void addConstraint(std::unique_ptr<FICAVCARAConstraint> C) {
    if (C)
      Constraints.push_back(std::move(C));
  }

private:
  std::vector<std::unique_ptr<FICAVCARAConstraint>> Constraints;

  void anchor() override;
};

} // end namespace llvm

#endif // LLVM_CODEGEN_FICAVCARACONSTRAINT_H
