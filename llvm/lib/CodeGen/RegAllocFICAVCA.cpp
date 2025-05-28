//===-- RegAllocBasic.cpp - Basic Register Allocator ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the RegAllocFICAVCA function pass, which provides a minimal
// implementation of the basic register allocator.
//
//===----------------------------------------------------------------------===//

#include "RegAllocFICAVCA.h"
#include "RegisterCoalescer.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/CalcSpillWeights.h"
#include "llvm/CodeGen/FICAVCARAConstraint.h"
#include "llvm/CodeGen/LiveInterval.h"
#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/LiveRangeEdit.h"
#include "llvm/CodeGen/LiveStacks.h"
#include "llvm/CodeGen/MachineBlockFrequencyInfo.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegAllocRegistry.h"
#include "llvm/CodeGen/SlotIndexes.h"
#include "llvm/CodeGen/Spiller.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/CodeGen/VirtRegMap.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Printable.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>

using namespace llvm;
using namespace FICAVCA;
using namespace RegAlloc;

#define DEBUG_TYPE "regalloc"

static RegisterRegAlloc
    RegisterFICAVCARepAlloc("ficavca", "FICAVCA register allocator",
                            createDefaultFICAVCARegisterAllocator);

#ifndef NDEBUG
static cl::opt<bool> PBQPDumpGraphs(
    "ficavca-dump-graphs",
    cl::desc("Dump graphs for each function/round in the compilation unit."),
    cl::init(false), cl::Hidden);
#endif

namespace llvm {
void printGraph(VirtRegMap &VRM, MachineFunction &MF, LiveIntervals &LIS,
                const char *Name);
}
namespace {

/// RegAllocFICAVCA provides a minimal implementation of the basic register
/// allocation algorithm. It prioritizes live virtual registers by spill weight
/// and spills whenever a register is unavailable. This is not practical in
/// production but provides a useful baseline both for measuring other
/// allocators and comparing the speed of the basic algorithm against other
/// styles of allocators.
class RegAllocFICAVCA : public MachineFunctionPass {
public:
  static char ID;

  RegAllocFICAVCA(char *customPassID = nullptr)
      : MachineFunctionPass(ID), customPassID(customPassID) {
    initializeSlotIndexesWrapperPassPass(*PassRegistry::getPassRegistry());
    initializeLiveIntervalsWrapperPassPass(*PassRegistry::getPassRegistry());
    initializeLiveStacksWrapperLegacyPass(*PassRegistry::getPassRegistry());
    initializeVirtRegMapWrapperLegacyPass(*PassRegistry::getPassRegistry());
  }

  /// Return the pass name.
  StringRef getPassName() const override {
    return "FICAVCA Register Allocator";
  }

  /// FICAVCA analysis usage.
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  /// Perform register allocation.
  bool runOnMachineFunction(MachineFunction &mf) override;

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().set(
        MachineFunctionProperties::Property::NoPHIs);
  }

  MachineFunctionProperties getClearedProperties() const override {
    return MachineFunctionProperties().set(
        MachineFunctionProperties::Property::IsSSA);
  }

private:
  using RegSet = std::set<Register>;

  char *customPassID;

  RegSet VRegsToAlloc, EmptyIntervalVRegs;

  SmallPtrSet<MachineInstr *, 32> DeadRemats;

  void findVRegIntervalsToAlloc(const MachineFunction &MF, LiveIntervals &LIS);

  void initializeGraph(FICAVCARAGraph &G, VirtRegMap &VRM,
                       Spiller &VRegSpiller);

  void spillVReg(Register VReg, SmallVectorImpl<Register> &NewIntervals,
                 MachineFunction &MF, LiveIntervals &LIS, VirtRegMap &VRM,
                 Spiller &VRegSpiller);

  bool mapFICAVCAToRegAlloc(const FICAVCARAGraph &G, const Solution &Solution,
                            VirtRegMap &VRM, Spiller &VRegSpiller);

  void finalizeAlloc(MachineFunction &MF, LiveIntervals &LIS,
                     VirtRegMap &VRM) const;

  void postOptimization(Spiller &VRegSpiller, LiveIntervals &LIS);
};

char RegAllocFICAVCA::ID = 0;

} // end anonymous namespace

/// Set spill costs for each node in the PBQP reg-alloc graph.
class SpillCosts : public FICAVCARAConstraint {
public:
  void apply(FICAVCARAGraph &G) override {
    LiveIntervals &LIS = G.getMetadata().LIS;
    for (auto NId : G.nodeIds()) {
      Float SpillCost =
          LIS.getInterval(G.getNodeMetadata(NId).getVReg()).weight();
      NodeMetadata &NM = G.getNodeMetadata(NId);
      NM.setSpillCost(SpillCost);
    }
  }
};

/// Add interference edges between overlapping vregs.
class Interference : public FICAVCARAConstraint {
private:
  using NodeId = GraphBase::NodeId;
  using EdgeId = GraphBase::EdgeId;
  using AllowedRegVecPtr = const AllowedRegVector *;
  using IKey = std::pair<AllowedRegVecPtr, AllowedRegVecPtr>;
  using DisjointAllowedRegsCache = DenseSet<IKey>;
  using IEdgeKey = std::pair<NodeId, NodeId>;
  using IEdgeCache = DenseSet<IEdgeKey>;

  bool haveDisjointAllowedRegs(const FICAVCARAGraph &G, NodeId NId, NodeId MId,
                               const DisjointAllowedRegsCache &D) const {
    const auto *NRegs = &G.getNodeMetadata(NId).getAllowedRegs();
    const auto *MRegs = &G.getNodeMetadata(MId).getAllowedRegs();

    if (NRegs == MRegs)
      return false;

    if (NRegs < MRegs)
      return D.contains(IKey(NRegs, MRegs));

    return D.contains(IKey(MRegs, NRegs));
  }

  void setDisjointAllowedRegs(const FICAVCARAGraph &G, NodeId NId, NodeId MId,
                              DisjointAllowedRegsCache &D) {
    const auto *NRegs = &G.getNodeMetadata(NId).getAllowedRegs();
    const auto *MRegs = &G.getNodeMetadata(MId).getAllowedRegs();

    assert(NRegs != MRegs && "AllowedRegs can not be disjoint with itself");

    if (NRegs < MRegs)
      D.insert(IKey(NRegs, MRegs));
    else
      D.insert(IKey(MRegs, NRegs));
  }

  // Holds (Interval, CurrentSegmentID, and NodeId). The first two are required
  // for the fast interference graph construction algorithm. The last is there
  // to save us from looking up node ids via the VRegToNode map in the graph
  // metadata.
  using IntervalInfo = std::tuple<LiveInterval *, size_t, GraphBase::NodeId>;

  static SlotIndex getStartPoint(const IntervalInfo &I) {
    return std::get<0>(I)->segments[std::get<1>(I)].start;
  }

  static SlotIndex getEndPoint(const IntervalInfo &I) {
    return std::get<0>(I)->segments[std::get<1>(I)].end;
  }

  static GraphBase::NodeId getNodeId(const IntervalInfo &I) {
    return std::get<2>(I);
  }

  static bool lowestStartPoint(const IntervalInfo &I1, const IntervalInfo &I2) {
    // Condition reversed because priority queue has the *highest* element at
    // the front, rather than the lowest.
    return getStartPoint(I1) > getStartPoint(I2);
  }

  static bool lowestEndPoint(const IntervalInfo &I1, const IntervalInfo &I2) {
    SlotIndex E1 = getEndPoint(I1);
    SlotIndex E2 = getEndPoint(I2);

    if (E1 < E2)
      return true;

    if (E1 > E2)
      return false;

    // If two intervals end at the same point, we need a way to break the tie or
    // the set will assume they're actually equal and refuse to insert a
    // "duplicate". Just compare the vregs - fast and guaranteed unique.
    return std::get<0>(I1)->reg() < std::get<0>(I2)->reg();
  }

  static bool isAtLastSegment(const IntervalInfo &I) {
    return std::get<1>(I) == std::get<0>(I)->size() - 1;
  }

  static IntervalInfo nextSegment(const IntervalInfo &I) {
    return std::make_tuple(std::get<0>(I), std::get<1>(I) + 1, std::get<2>(I));
  }

public:
  void apply(FICAVCARAGraph &G) override {
    // The following is loosely based on the linear scan algorithm introduced in
    // "Linear Scan Register Allocation" by Poletto and Sarkar. This version
    // isn't linear, because the size of the active set isn't bound by the
    // number of registers, but rather the size of the largest clique in the
    // graph. Still, we expect this to be better than N^2.
    LiveIntervals &LIS = G.getMetadata().LIS;

    // Finding an edge is expensive in the worst case (O(max_clique(G))). So
    // cache locally edges we have already seen.
    IEdgeCache EC;

    // Cache known disjoint allowed registers pairs
    DisjointAllowedRegsCache D;

    using IntervalSet = std::set<IntervalInfo, decltype(&lowestEndPoint)>;
    using IntervalQueue =
        std::priority_queue<IntervalInfo, std::vector<IntervalInfo>,
                            decltype(&lowestStartPoint)>;
    IntervalSet Active(lowestEndPoint);
    IntervalQueue Inactive(lowestStartPoint);

    // Start by building the inactive set.
    for (auto NId : G.nodeIds()) {
      Register VReg = G.getNodeMetadata(NId).getVReg();
      LiveInterval &LI = LIS.getInterval(VReg);
      assert(!LI.empty() && "PBQP graph contains node for empty interval");
      Inactive.push(std::make_tuple(&LI, 0, NId));
    }

    while (!Inactive.empty()) {
      // Tentatively grab the "next" interval - this choice may be overriden
      // below.
      IntervalInfo Cur = Inactive.top();

      // Retire any active intervals that end before Cur starts.
      IntervalSet::iterator RetireItr = Active.begin();
      while (RetireItr != Active.end() &&
             (getEndPoint(*RetireItr) <= getStartPoint(Cur))) {
        // If this interval has subsequent segments, add the next one to the
        // inactive list.
        if (!isAtLastSegment(*RetireItr))
          Inactive.push(nextSegment(*RetireItr));

        ++RetireItr;
      }
      Active.erase(Active.begin(), RetireItr);

      // One of the newly retired segments may actually start before the
      // Cur segment, so re-grab the front of the inactive list.
      Cur = Inactive.top();
      Inactive.pop();

      // At this point we know that Cur overlaps all active intervals. Add the
      // interference edges.
      GraphBase::NodeId NId = getNodeId(Cur);
      for (const auto &A : Active) {
        GraphBase::NodeId MId = getNodeId(A);

        // Do not add an edge when the nodes' allowed registers do not
        // intersect: there is obviously no interference.
        if (haveDisjointAllowedRegs(G, NId, MId, D))
          continue;

        // Check that we haven't already added this edge
        IEdgeKey EK(std::min(NId, MId), std::max(NId, MId));
        if (EC.count(EK))
          continue;

        // This is a new edge - add it to the graph.
        if (!createInterferenceEdge(G, NId, MId))
          setDisjointAllowedRegs(G, NId, MId, D);
        else
          EC.insert(EK);
      }

      // Finally, add Cur to the Active set.
      Active.insert(Cur);
    }
  }

private:
  // Create an Interference edge and add it to the graph, unless it is
  // a null matrix, meaning the nodes' allowed registers do not have any
  // interference. This case occurs frequently between integer and floating
  // point registers for example.
  // return true iff both nodes interferes.
  bool createInterferenceEdge(FICAVCARAGraph &G, NodeId NId, NodeId MId) {
    const TargetRegisterInfo &TRI =
        *G.getMetadata().MF.getSubtarget().getRegisterInfo();
    const auto &NRegs = G.getNodeMetadata(NId).getAllowedRegs();
    const auto &MRegs = G.getNodeMetadata(MId).getAllowedRegs();
    // outs() << "NId " << NId << " size " << NRegs.size() << "; MId " << MId
    //        << " size " << MRegs.size() << "\n";
    bool NodesInterfere = false;
    for (unsigned I = 0; I != NRegs.size(); ++I) {
      MCRegister PRegN = NRegs[I];
      for (unsigned J = 0; J != MRegs.size(); ++J) {
        MCRegister PRegM = MRegs[J];
        if (TRI.regsOverlap(PRegN, PRegM)) {
          NodesInterfere = true;
        }
      }
    }

    if (!NodesInterfere)
      return false;

    G.addEdge(NId, MId);
    return true;
  }
};

FICAVCARAConstraint::~FICAVCARAConstraint() = default;

void FICAVCARAConstraint::anchor() {}

void FICAVCARAConstraintList::anchor() {}

void RegAllocFICAVCA::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  AU.addRequired<AAResultsWrapperPass>();
  AU.addPreserved<AAResultsWrapperPass>();
  AU.addRequired<SlotIndexesWrapperPass>();
  AU.addPreserved<SlotIndexesWrapperPass>();
  AU.addRequired<LiveIntervalsWrapperPass>();
  AU.addPreserved<LiveIntervalsWrapperPass>();
  AU.addRequired<LiveStacksWrapperLegacy>();
  AU.addPreserved<LiveStacksWrapperLegacy>();
  AU.addRequired<MachineBlockFrequencyInfoWrapperPass>();
  AU.addPreserved<MachineBlockFrequencyInfoWrapperPass>();
  AU.addRequired<MachineLoopInfoWrapperPass>();
  AU.addPreserved<MachineLoopInfoWrapperPass>();
  AU.addRequired<MachineDominatorTreeWrapperPass>();
  AU.addPreserved<MachineDominatorTreeWrapperPass>();
  AU.addRequired<VirtRegMapWrapperLegacy>();
  AU.addPreserved<VirtRegMapWrapperLegacy>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

void RegAllocFICAVCA::findVRegIntervalsToAlloc(const MachineFunction &MF,
                                               LiveIntervals &LIS) {
  const MachineRegisterInfo &MRI = MF.getRegInfo();

  for (unsigned I = 0, E = MRI.getNumVirtRegs(); I != E; ++I) {
    Register Reg = Register::index2VirtReg(I);
    if (MRI.reg_nodbg_empty(Reg))
      continue;
    VRegsToAlloc.insert(Reg);
  }
}

void RegAllocFICAVCA::initializeGraph(FICAVCARAGraph &G, VirtRegMap &VRM,
                                      Spiller &VRegSpiller) {
  MachineFunction &MF = G.getMetadata().MF;

  LiveIntervals &LIS = G.getMetadata().LIS;
  const MachineRegisterInfo &MRI = G.getMetadata().MF.getRegInfo();
  const TargetRegisterInfo &TRI =
      *G.getMetadata().MF.getSubtarget().getRegisterInfo();

  std::vector<Register> Worklist(VRegsToAlloc.begin(), VRegsToAlloc.end());

  std::map<Register, std::vector<MCRegister>> VRegAllowedMap;

  while (!Worklist.empty()) {
    Register VReg = Worklist.back();
    Worklist.pop_back();

    LiveInterval &VRegLI = LIS.getInterval(VReg);

    // If this is an empty interval move it to the EmptyIntervalVRegs set then
    // continue.
    if (VRegLI.empty()) {
      EmptyIntervalVRegs.insert(VRegLI.reg());
      VRegsToAlloc.erase(VRegLI.reg());
      continue;
    }

    const TargetRegisterClass *TRC = MRI.getRegClass(VReg);

    // Record any overlaps with regmask operands.
    BitVector RegMaskOverlaps;
    LIS.checkRegMaskInterference(VRegLI, RegMaskOverlaps);

    // Compute an initial allowed set for the current vreg.
    std::vector<MCRegister> VRegAllowed;
    ArrayRef<MCPhysReg> RawPRegOrder = TRC->getRawAllocationOrder(MF);
    for (MCPhysReg R : RawPRegOrder) {
      MCRegister PReg(R);
      if (MRI.isReserved(PReg))
        continue;

      // vregLI crosses a regmask operand that clobbers preg.
      if (!RegMaskOverlaps.empty() && !RegMaskOverlaps.test(PReg))
        continue;

      // vregLI overlaps fixed regunit interference.
      bool Interference = false;
      for (MCRegUnit Unit : TRI.regunits(PReg)) {
        if (VRegLI.overlaps(LIS.getRegUnit(Unit))) {
          Interference = true;
          break;
        }
      }
      if (Interference)
        continue;

      // preg is usable for this virtual register.
      VRegAllowed.push_back(PReg);
    }

    // Check for vregs that have no allowed registers. These should be
    // pre-spilled and the new vregs added to the worklist.
    if (VRegAllowed.empty()) {
      SmallVector<Register, 8> NewVRegs;
      spillVReg(VReg, NewVRegs, MF, LIS, VRM, VRegSpiller);
      llvm::append_range(Worklist, NewVRegs);
      continue;
    }

    VRegAllowedMap[VReg.id()] = std::move(VRegAllowed);
  }

  for (auto &KV : VRegAllowedMap) {
    auto VReg = KV.first;

    // Move empty intervals to the EmptyIntervalVReg set.
    if (LIS.getInterval(VReg).empty()) {
      EmptyIntervalVRegs.insert(VReg);
      VRegsToAlloc.erase(VReg);
      continue;
    }

    auto &VRegAllowed = KV.second;

    GraphBase::NodeId NId = G.addNode();
    G.getNodeMetadata(NId).setVReg(VReg);
    G.getNodeMetadata(NId).setAllowedRegs(
        G.getMetadata().getAllowedRegs(std::move(VRegAllowed)));
    G.getMetadata().setNodeIdForVReg(VReg, NId);
  }
}

void RegAllocFICAVCA::spillVReg(Register VReg,
                                SmallVectorImpl<Register> &NewIntervals,
                                MachineFunction &MF, LiveIntervals &LIS,
                                VirtRegMap &VRM, Spiller &VRegSpiller) {
  VRegsToAlloc.erase(VReg);
  LiveRangeEdit LRE(&LIS.getInterval(VReg), NewIntervals, MF, LIS, &VRM,
                    nullptr, &DeadRemats);
  VRegSpiller.spill(LRE);

  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  (void)TRI;
  LLVM_DEBUG(dbgs() << "VREG " << printReg(VReg, &TRI) << " -> SPILLED (Cost: "
                    << LRE.getParent().weight() << ", New vregs: ");

  // Copy any newly inserted live intervals into the list of regs to
  // allocate.
  for (const Register &R : LRE) {
    const LiveInterval &LI = LIS.getInterval(R);
    assert(!LI.empty() && "Empty spill range.");
    LLVM_DEBUG(dbgs() << printReg(LI.reg(), &TRI) << " ");
    VRegsToAlloc.insert(LI.reg());
  }

  LLVM_DEBUG(dbgs() << ")\n");
}

bool RegAllocFICAVCA::mapFICAVCAToRegAlloc(const FICAVCARAGraph &G,
                                           const Solution &Solution,
                                           VirtRegMap &VRM,
                                           Spiller &VRegSpiller) {
  MachineFunction &MF = G.getMetadata().MF;
  LiveIntervals &LIS = G.getMetadata().LIS;
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  (void)TRI;

  // Set to true if we have any spills
  bool AnotherRoundNeeded = false;

  // Clear the existing allocation.
  VRM.clearAllVirt();

  // Iterate over the nodes mapping the PBQP solution to a register
  // assignment.
  for (auto NId : G.nodeIds()) {
    Register VReg = G.getNodeMetadata(NId).getVReg();
    unsigned AllocOpt = Solution.getSelection(NId);

    if (AllocOpt != getSpillOptionIdx()) {
      MCRegister PReg = G.getNodeMetadata(NId).getAllowedRegs()[AllocOpt - 1];
      LLVM_DEBUG(dbgs() << "VREG " << printReg(VReg, &TRI) << " -> "
                        << TRI.getName(PReg) << "(" << PReg.id() << ")\n");
      assert(PReg != 0 && "Invalid preg selected.");
      VRM.assignVirt2Phys(VReg, PReg);
    } else {
      // Spill VReg. If this introduces new intervals we'll need another round
      // of allocation.
      SmallVector<Register, 8> NewVRegs;
      spillVReg(VReg, NewVRegs, MF, LIS, VRM, VRegSpiller);
      AnotherRoundNeeded |= !NewVRegs.empty();
    }
  }

  return !AnotherRoundNeeded;
}

void RegAllocFICAVCA::finalizeAlloc(MachineFunction &MF, LiveIntervals &LIS,
                                    VirtRegMap &VRM) const {
  MachineRegisterInfo &MRI = MF.getRegInfo();

  // First allocate registers for the empty intervals.
  for (const Register &R : EmptyIntervalVRegs) {
    LiveInterval &LI = LIS.getInterval(R);

    Register PReg = MRI.getSimpleHint(LI.reg());

    if (PReg == 0) {
      const TargetRegisterClass &RC = *MRI.getRegClass(LI.reg());
      const ArrayRef<MCPhysReg> RawPRegOrder = RC.getRawAllocationOrder(MF);
      for (MCRegister CandidateReg : RawPRegOrder) {
        if (!VRM.getRegInfo().isReserved(CandidateReg)) {
          PReg = CandidateReg;
          break;
        }
      }
      assert(PReg &&
             "No un-reserved physical registers in this register class");
    }

    VRM.assignVirt2Phys(LI.reg(), PReg);
  }
}

void RegAllocFICAVCA::postOptimization(Spiller &VRegSpiller,
                                       LiveIntervals &LIS) {
  VRegSpiller.postOptimization();
  /// Remove dead defs because of rematerialization.
  for (auto *DeadInst : DeadRemats) {
    LIS.RemoveMachineInstrFromMaps(*DeadInst);
    DeadInst->eraseFromParent();
  }
  DeadRemats.clear();
}

bool RegAllocFICAVCA::runOnMachineFunction(MachineFunction &MF) {
  LiveIntervals &LIS = getAnalysis<LiveIntervalsWrapperPass>().getLIS();
  MachineBlockFrequencyInfo &MBFI =
      getAnalysis<MachineBlockFrequencyInfoWrapperPass>().getMBFI();

  auto &LiveStks = getAnalysis<LiveStacksWrapperLegacy>().getLS();
  auto &MDT = getAnalysis<MachineDominatorTreeWrapperPass>().getDomTree();

  VirtRegMap &VRM = getAnalysis<VirtRegMapWrapperLegacy>().getVRM();

  VirtRegAuxInfo VRAI(MF, LIS, VRM,
                      getAnalysis<MachineLoopInfoWrapperPass>().getLI(), MBFI);

  VRAI.calculateSpillWeightsAndHints();

  // FIXME: we create DefaultVRAI here to match existing behavior pre-passing
  // the VRAI through the spiller to the live range editor. However, it probably
  // makes more sense to pass the PBQP VRAI. The existing behavior had
  // LiveRangeEdit make its own VirtRegAuxInfo object.
  VirtRegAuxInfo DefaultVRAI(
      MF, LIS, VRM, getAnalysis<MachineLoopInfoWrapperPass>().getLI(), MBFI);
  std::unique_ptr<Spiller> VRegSpiller(
      createInlineSpiller({LIS, LiveStks, MDT, MBFI}, MF, VRM, DefaultVRAI));

  MF.getRegInfo().freezeReservedRegs();

  LLVM_DEBUG(dbgs() << "FICAVCA Register Allocating for " << MF.getName()
                    << "\n");

  // Allocator main loop:
  //
  // * Map current regalloc problem to a PBQP problem
  // * Solve the PBQP problem
  // * Map the solution back to a register allocation
  // * Spill if necessary
  //
  // This process is continued till no more spills are generated.

  // Find the vreg intervals in need of allocation.
  findVRegIntervalsToAlloc(MF, LIS);

#ifndef NDEBUG
  const Function &F = MF.getFunction();
  std::string FullyQualifiedName =
      F.getParent()->getModuleIdentifier() + "." + F.getName().str();
#endif

  // If there are non-empty intervals allocate them using pbqp.
  if (!VRegsToAlloc.empty()) {
    // const TargetSubtargetInfo &Subtarget = MF.getSubtarget();
    std::unique_ptr<FICAVCARAConstraintList> ConstraintsRoot =
        std::make_unique<FICAVCARAConstraintList>();
    ConstraintsRoot->addConstraint(std::make_unique<SpillCosts>());
    ConstraintsRoot->addConstraint(std::make_unique<Interference>());

    bool PBQPAllocComplete = false;
    unsigned Round = 0;

    while (!PBQPAllocComplete) {
      (void)Round;

      FICAVCARAGraph G(FICAVCARAGraph::GraphMetadata(MF, LIS, MBFI));
      initializeGraph(G, VRM, *VRegSpiller);

      ConstraintsRoot->apply(G);

#ifndef NDEBUG
      if (1) {
        std::ostringstream RS;
        RS << Round;
        std::string GraphFileName =
            FullyQualifiedName + "." + RS.str() + ".ficavcagraph";
        std::error_code EC;
        raw_fd_ostream OS(GraphFileName, EC, sys::fs::OF_TextWithCRLF);
        LLVM_DEBUG(dbgs() << "Dumping graph for round " << Round << " to \""
                          << GraphFileName << "\"\n");
        G.dump(OS);
      }
#endif

      Solution Solution = solve(G);
      PBQPAllocComplete = mapFICAVCAToRegAlloc(G, Solution, VRM, *VRegSpiller);
      ++Round;
    }
  }

  // Finalise allocation, allocate empty ranges.
  finalizeAlloc(MF, LIS, VRM);
  printGraph(VRM, MF, LIS, "ficavca");
  postOptimization(*VRegSpiller, LIS);
  VRegsToAlloc.clear();
  EmptyIntervalVRegs.clear();

  LLVM_DEBUG(dbgs() << "Post alloc VirtRegMap:\n" << VRM << "\n");

  return true;
}

FunctionPass *llvm::createFICAVCARegisterAllocator(char *customPassID) {
  return new RegAllocFICAVCA(customPassID);
}

FunctionPass *llvm::createDefaultFICAVCARegisterAllocator() {
  return createFICAVCARegisterAllocator();
}
