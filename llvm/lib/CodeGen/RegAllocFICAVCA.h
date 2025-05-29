//===- RegAllocFICAVCA.h ----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the FICAVCABuilder interface, for classes which build
// FICAVCA instances to represent register allocation problems, and the
// RegAllocFICAVCA interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_REGALLOCFICAVCA_H
#define LLVM_CODEGEN_REGALLOCFICAVCA_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/CodeGen/FICAVCA/Graph.h"
#include "llvm/CodeGen/PBQP/CostAllocator.h"
#include "llvm/CodeGen/PBQP/Solution.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <list>
#include <memory>
#include <set>
#include <unordered_set>
#include <vector>

namespace llvm {

class FunctionPass;
class LiveIntervals;
class MachineBlockFrequencyInfo;
class MachineFunction;
class raw_ostream;

namespace FICAVCA {
namespace RegAlloc {

template <typename SolverTy> using Graph = FICAVCA::Graph<SolverTy>;
template <typename ValueT> using ValuePool = PBQP::ValuePool<ValueT>;
using GraphBase = PBQP::GraphBase;
using Solution = PBQP::Solution;
using Float = float;

/// Spill option index.
inline unsigned getSpillOptionIdx() { return 0; }
struct hash_value;
/// Holds a vector of the allowed physical regs for a vreg.
class AllowedRegVector : std::vector<MCRegister> {
  using BaseVector = std::vector<MCRegister>;

  friend struct hash_value;

public:
  AllowedRegVector() = default;
  AllowedRegVector(const AllowedRegVector &) = delete;
  AllowedRegVector& operator=(const AllowedRegVector &) = delete;
  AllowedRegVector(AllowedRegVector &&) = default;
  AllowedRegVector& operator=(AllowedRegVector &&) = default;

  AllowedRegVector(const std::vector<MCRegister> &OptVec)
      : BaseVector(OptVec) {}

  using BaseVector::begin;
  using BaseVector::end;
  using BaseVector::size;
  using BaseVector::operator[];

  bool operator==(const AllowedRegVector &Other) const {
    if (size() != Other.size())
      return false;
    return std::equal(begin(), end(), Other.begin());
  }

  bool operator!=(const AllowedRegVector &Other) const {
    return !(*this == Other);
  }

private:
  std::vector<MCRegister> Opts;
};

struct hash_value {
hash_code operator()(const AllowedRegVector &OptRegs) const {
  assert(OptRegs.size() > 0 && "OptRegs is empty");
  const MCRegister *OStart = &OptRegs.front();
  const MCRegister *OEnd = &OptRegs.back();
  return hash_combine(OptRegs.size(), hash_combine_range(OStart, OEnd));
}
};

/// Holds graph-level metadata relevant to FICAVCA RA problems.
class GraphMetadata {
  //using AllowedRegVecPool = ValuePool<AllowedRegVector>;

public:
  using NodeId = GraphBase::NodeId;
  using AllowedRegVecRef = const AllowedRegVector*;

  GraphMetadata(MachineFunction &MF, LiveIntervals &LIS,
                MachineBlockFrequencyInfo &MBFI)
      : MF(MF), LIS(LIS), MBFI(MBFI) {}

  MachineFunction &MF;
  LiveIntervals &LIS;
  MachineBlockFrequencyInfo &MBFI;

  void setNodeIdForVReg(Register VReg, NodeId NId) {
    VRegToNodeId[VReg.id()] = NId;
  }

  NodeId getNodeIdForVReg(Register VReg) const {
    auto VRegItr = VRegToNodeId.find(VReg);
    if (VRegItr == VRegToNodeId.end())
      return GraphBase::invalidNodeId();
    return VRegItr->second;
  }

  AllowedRegVecRef getAllowedRegs(AllowedRegVector Allowed) {
    auto Found = AllowedRegVecs.find(Allowed);
    if (Found != AllowedRegVecs.end()) {
      return std::addressof(*Found);
    } else {
      auto [Inserted, Flag] = AllowedRegVecs.emplace(std::move(Allowed));
      return std::addressof(*Inserted);
    }
  }

private:
  DenseMap<Register, NodeId> VRegToNodeId;
  std::unordered_set<AllowedRegVector, hash_value> AllowedRegVecs;
};

/// Holds solver state and other metadata relevant to each FICAVCA RA node.
class NodeMetadata {
public:
  NodeMetadata() = default;

  using AllowedRegVector = RegAlloc::AllowedRegVector;
  using AllowedRegVecRef = GraphMetadata::AllowedRegVecRef;

  void setVReg(Register VR) { VReg = VR; }

  Register getVReg() const { return VReg; }

  void setAllowedRegs(AllowedRegVecRef AR) { AllowedRegs = AR; }

  const AllowedRegVector &getAllowedRegs() const { return *AllowedRegs; }

  void setSpillCost(Float SC) { SpillCost = SC; }

  Float getSpillCost() const { return SpillCost; }

  bool Colored;
  unsigned Color;
  Float Purity;
  unsigned VoteWeight;

private:
  Register VReg;
  Float SpillCost;
  AllowedRegVecRef AllowedRegs;
};

class RegAllocSolverImpl {
public:
  using NodeId = GraphBase::NodeId;
  using EdgeId = GraphBase::EdgeId;

  using NodeMetadata = RegAlloc::NodeMetadata;
  struct EdgeMetadata {};
  using GraphMetadata = RegAlloc::GraphMetadata;
  using GraphTy = Graph<RegAllocSolverImpl>;

  RegAllocSolverImpl(GraphTy &G) : G(G) {}

  Solution solve() {
    initialize();
    for (unsigned Stage = 0;; ++Stage) {
      vote();
      color();
      update();
      if (UncoloredVerteces.empty()) {
        break;
      }
    }
    Solution S;
    for (NodeId NId : G.nodeIds()) {
      const NodeMetadata &NM = G.getNodeMetadata(NId);
      S.setSelection(NId, NM.Color);
    }
    // for (auto EId : G.edgeIds()) {
    //   outs() << G.getEdgeNode1Id(EId) << " - " << G.getEdgeNode2Id(EId) << "\n";
    // }
    return S;
  }

  void setDemocracy(bool D) { Democracy = D; }

  bool validate() const {
    GraphTy::EdgeIdSet EIds = G.edgeIds();
    return std::all_of(EIds.begin(), EIds.end(), [&](EdgeId EId) {
      NodeId N1Id = G.getEdgeNode1Id(EId);
      NodeId N2Id = G.getEdgeNode2Id(EId);
      NodeMetadata &NM1 = G.getNodeMetadata(N1Id);
      NodeMetadata &NM2 = G.getNodeMetadata(N2Id);
      if (NM1.Colored && NM2.Colored && (NM1.Color == getSpillOptionIdx() || NM2.Color == getSpillOptionIdx()))
        return true;
      return NM1.Colored && NM2.Colored && NM1.getAllowedRegs()[NM1.Color-1] != NM2.getAllowedRegs()[NM2.Color-1];
    });
  }

  unsigned getColorDegree() const { return UsedColors.size(); }

private:
  GraphTy &G;
  bool Democracy = true;
  std::vector<NodeId> NominateList;
  std::list<NodeId> UncoloredVerteces;
  std::unordered_set<unsigned> UsedColors;

  void initialize() {
    GraphTy::NodeIdSet NIds = G.nodeIds();
    std::transform(NIds.begin(), NIds.end(),
                   std::back_inserter(UncoloredVerteces), [&](NodeId NId) {
                     NodeMetadata &NM = G.getNodeMetadata(NId);
                     NM.Color = 0;
                     NM.Colored = false;
                     NM.Purity = Float{1};
                     NM.VoteWeight = 0;
                     return NId;
                   });
  }

  void vote() {
    NominateList.clear();
    for (NodeId NId : UncoloredVerteces) {
      NodeMetadata &NM = G.getNodeMetadata(NId);
      NM.VoteWeight = 0;
    }
    for (auto NId : UncoloredVerteces) {
      const NodeMetadata &NM = G.getNodeMetadata(NId);
      if (!(NM.Purity > 0.001)) {
        continue;
      }
      GraphTy::AdjEdgeIdSet EIds = G.adjEdgeIds(NId);
      bool HasSuperior = false;
      bool PurityCompare = false;
      std::for_each(EIds.begin(), EIds.end(), [&](EdgeId EId) {
        NodeId AdjNId = G.getEdgeOtherNodeId(EId, NId);
        GraphTy::AdjEdgeIdSet AdjEIds = G.adjEdgeIds(AdjNId);
        NodeMetadata &AdjNM = G.getNodeMetadata(AdjNId);
        if (Democracy) {
          PurityCompare = AdjNM.Purity >= NM.Purity;
        } else {
          PurityCompare = AdjNM.Purity > NM.Purity;
        }
        if (!AdjNM.Colored && (PurityCompare && AdjEIds.size() > EIds.size())) {
          AdjNM.VoteWeight += EIds.size();
          HasSuperior = true;
        }
      });
      if (!HasSuperior) {
        NominateList.emplace_back(NId);
      }
    }
  }

  void color() {
    if (!NominateList.empty()) {
      for (NodeId NId : NominateList) {
        colorVertex(NId);
      }
    } else {
      assert(!UncoloredVerteces.empty());
      auto HighestVote =
          std::max_element(UncoloredVerteces.begin(), UncoloredVerteces.end(),
                           [&](NodeId N1Id, NodeId N2Id) {
                             const NodeMetadata &NN1 = G.getNodeMetadata(N1Id);
                             const NodeMetadata &NM2 = G.getNodeMetadata(N2Id);
                             return NN1.VoteWeight < NM2.VoteWeight;
                           });
      colorVertex(*HighestVote);
    }
    for (NodeId NId : UncoloredVerteces) {
      const NodeMetadata &NM = G.getNodeMetadata(NId);
      if (NM.Purity < 0.001) {
        colorVertex(NId);
      }
    }
  }

  void colorVertex(NodeId NId) {
    const MachineRegisterInfo &MRI = G.getMetadata().MF.getRegInfo();
    const TargetRegisterInfo &TRI = *MRI.getTargetRegisterInfo();

    NodeMetadata &NM = G.getNodeMetadata(NId);
    GraphTy::AdjEdgeIdSet EIds = G.adjEdgeIds(NId);
    std::unordered_set<unsigned> AdjColors;
    for (EdgeId EId : EIds) {
      NodeId AdjNId = G.getEdgeOtherNodeId(EId, NId);
      NodeMetadata &AdjNM = G.getNodeMetadata(AdjNId);
      if (AdjNM.Colored && AdjNM.Color != getSpillOptionIdx()) {
        const AllowedRegVector &AdjAllowedRegs = AdjNM.getAllowedRegs();
        AdjColors.emplace(AdjAllowedRegs[AdjNM.Color - 1].id());
      }
    }
    const AllowedRegVector &AllowedRegs = NM.getAllowedRegs();
    auto Found = std::find_if(AllowedRegs.begin(), AllowedRegs.end(),
                              [&](const MCRegister &PReg) {
                                unsigned PRegId = PReg.id();
                                bool Overlap = std::any_of(AdjColors.begin(), AdjColors.end(), [&](unsigned AdjPReg) {
                                  return TRI.regsOverlap(PRegId, AdjPReg);
                                });
                                return !Overlap && AdjColors.count(PReg.id()) == 0;
                              });
    if (Found != AllowedRegs.end()) {
      NM.Color = std::distance(AllowedRegs.begin(), Found) + 1;
      UsedColors.emplace(Found->id());
    } else {
      NM.Color = getSpillOptionIdx();
    }
    NM.Colored = true;
  }

  void update() {
    for (NodeId NId : UncoloredVerteces) {
      updatePurity(NId);
    }
    UncoloredVerteces.remove_if([&](NodeId NId) {
      const NodeMetadata &NM = G.getNodeMetadata(NId);
      return NM.Colored;
    });
  }

  void updatePurity(NodeId NId) {
    NodeMetadata &NM = G.getNodeMetadata(NId);
    GraphTy::AdjEdgeIdSet EIds = G.adjEdgeIds(NId);
    auto Count = std::count_if(EIds.begin(), EIds.end(), [&](EdgeId EId) {
      NodeId AdjNId = G.getEdgeOtherNodeId(EId, NId);
      const NodeMetadata &AdjNM = G.getNodeMetadata(AdjNId);
      return !AdjNM.Colored;
    });
    NM.Purity = static_cast<Float>(Count) / EIds.size();
    if (NM.getSpillCost() > 1) {
      NM.Purity = 0;
    }
  }
};

class FICAVCARAGraph : public Graph<RegAllocSolverImpl> {
  using BaseTy = Graph<RegAllocSolverImpl>;

  void complicatedVertexDump(raw_ostream &OS, const NodeMetadata &NM) const {
    OS << "Colored " << NM.Colored << "|";
    OS << "ColorIdx " << NM.Color << "|";
    unsigned Color = NM.Color;
    if (Color != getSpillOptionIdx()) {
      Color = NM.getAllowedRegs()[Color-1].id();
    }
    OS << "Color " << Color << "|";
    OS << "Purity " << llvm::format("%.2f", NM.Purity) << "|";
    OS << "VoteWeight " << NM.VoteWeight << "|";
    const AllowedRegVector& AR = NM.getAllowedRegs();
    OS << "AllowedRegs [";
    std::string Separator;
    for (MCRegister MReg : AR) {
      OS << Separator << MReg.id();
      Separator = ", ";
    }
    OS << "] |";
  }

public:
  FICAVCARAGraph(GraphMetadata Metadata) : BaseTy(std::move(Metadata)) {}

  /// Dump this graph to dbgs().
  void dump() const {}

  /// Dump this graph to an output stream.
  /// @param OS Output stream to print on.
  void dump(raw_ostream &OS) const {}

  /// Print a representation of this graph in DOT format.
  /// @param OS Output stream to print on.
  void printDot(raw_ostream &OS) const {
    OS << "graph {\n";
    OS << "\trankdir=LR;\n";
    OS << "\tnode[shape=record, style=filled, fontcolor=black];\n";
    for (NodeId NId : nodeIds()) {
      const NodeMetadata &NM = getNodeMetadata(NId);
      OS << "\tnode_" << NId << "[label = \"Id " << NId << "|";
      complicatedVertexDump(OS, NM);
      OS << "Neighbors " << adjEdgeIds(NId).size() << "\"];\n";
    }
    for (auto Id : edgeIds()) {
      NodeId V1 = getEdgeNode1Id(Id);
      NodeId V2 = getEdgeNode2Id(Id);
      OS << "\tnode_" << V1 << " -- node_" << V2 << ";\n";
    }
    OS << "}\n";
  }

  /// Print a representation of this graph in simple format.
  /// @param OS Output stream to print on.
  void printSimple(raw_ostream &OS) const {}
};

inline Solution solve(FICAVCARAGraph &G) {
  if (G.empty())
    return Solution();
  RegAllocSolverImpl RegAllocSolver(G);
  Solution S = RegAllocSolver.solve();
  // outs() << "Validated: " << RegAllocSolver.validate() << "\n";
  return S;
}

} // end namespace RegAlloc

} // end namespace FICAVCA

/// Create a FICAVCA register allocator instance.
FunctionPass *
createFICAVCARegisterAllocator(char *customPassID = nullptr);

} // end namespace llvm

#endif // LLVM_CODEGEN_REGALLOCFICAVCA_H
