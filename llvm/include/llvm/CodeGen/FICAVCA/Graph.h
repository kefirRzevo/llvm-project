//===- Graph.h - FICAVCA Graph ----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// FICAVCA Graph class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_FICAVCA_GRAPH_H
#define LLVM_CODEGEN_FICAVCA_GRAPH_H

#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/PBQP/Graph.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <vector>

namespace llvm {
namespace FICAVCA {

template <typename SolverTy>
class Graph : public PBQP::GraphBase {
public:
  using GraphBase::invalidNodeId;
  using GraphBase::invalidEdgeId;
  using typename GraphBase::NodeId;
  using typename GraphBase::EdgeId;
  using NodeMetadata = typename SolverTy::NodeMetadata;
  using EdgeMetadata = typename SolverTy::EdgeMetadata;
  using GraphMetadata = typename SolverTy::GraphMetadata;

private:
  class NodeEntry {
  public:
    using AdjEdgeList = std::vector<EdgeId>;
    using AdjEdgeIdx = typename AdjEdgeList::size_type;
    using AdjEdgeItr = typename AdjEdgeList::const_iterator;

    template <typename... NodeMetadataArgs>
    NodeEntry(NodeMetadataArgs &&...Args)
        : Metadata(std::forward<NodeMetadataArgs>(Args)...) {}

    static AdjEdgeIdx getInvalidAdjEdgeIdx() {
      return std::numeric_limits<AdjEdgeIdx>::max();
    }

    AdjEdgeIdx addAdjEdgeId(EdgeId EId) {
      auto Idx = AdjEdgeIds.size();
      AdjEdgeIds.push_back(EId);
      return Idx;
    }

    void removeAdjEdgeId(Graph &G, NodeId ThisNId, AdjEdgeIdx Idx) {
      G.getEdge(AdjEdgeIds.back()).setAdjEdgeIdx(ThisNId, Idx);
      AdjEdgeIds[Idx] = AdjEdgeIds.back();
      AdjEdgeIds.pop_back();
    }

    const AdjEdgeList &getAdjEdgeIds() const { return AdjEdgeIds; }
    AdjEdgeItr adjEdgesBegin() const { return AdjEdgeIds.begin(); }
    AdjEdgeItr adjEdgesEnd() const { return AdjEdgeIds.end(); }

    NodeMetadata Metadata;

  private:
    AdjEdgeList AdjEdgeIds;
  };

  class EdgeEntry {
  public:
    using AdjEdgeIdx = typename NodeEntry::AdjEdgeIdx;

    template <typename... EdgeMetadataArgs>
    EdgeEntry(NodeId N1Id, NodeId N2Id, EdgeMetadataArgs &&...Args)
        : Metadata(std::forward<EdgeMetadataArgs>(Args)...) {
      NIds[0] = N1Id;
      NIds[1] = N2Id;
      ThisEdgeAdjIdxs[0] = NodeEntry::getInvalidAdjEdgeIdx();
      ThisEdgeAdjIdxs[1] = NodeEntry::getInvalidAdjEdgeIdx();
    }

    void connectToN(Graph &G, EdgeId ThisEdgeId, unsigned NIdx) {
      assert(ThisEdgeAdjIdxs[NIdx] == NodeEntry::getInvalidAdjEdgeIdx() &&
             "Edge already connected to NIds[NIdx].");
      NodeEntry &N = G.getNode(NIds[NIdx]);
      ThisEdgeAdjIdxs[NIdx] = N.addAdjEdgeId(ThisEdgeId);
    }

    void connect(Graph &G, EdgeId ThisEdgeId) {
      connectToN(G, ThisEdgeId, 0);
      connectToN(G, ThisEdgeId, 1);
    }

    void setAdjEdgeIdx(NodeId NId, AdjEdgeIdx NewIdx) {
      if (NId == NIds[0])
        ThisEdgeAdjIdxs[0] = NewIdx;
      else {
        assert(NId == NIds[1] && "Edge not connected to NId");
        ThisEdgeAdjIdxs[1] = NewIdx;
      }
    }

    void disconnectFromN(Graph &G, unsigned NIdx) {
      assert(ThisEdgeAdjIdxs[NIdx] != NodeEntry::getInvalidAdjEdgeIdx() &&
             "Edge not connected to NIds[NIdx].");
      NodeEntry &N = G.getNode(NIds[NIdx]);
      N.removeAdjEdgeId(G, NIds[NIdx], ThisEdgeAdjIdxs[NIdx]);
      ThisEdgeAdjIdxs[NIdx] = NodeEntry::getInvalidAdjEdgeIdx();
    }

    void disconnectFrom(Graph &G, NodeId NId) {
      if (NId == NIds[0])
        disconnectFromN(G, 0);
      else {
        assert(NId == NIds[1] && "Edge does not connect NId");
        disconnectFromN(G, 1);
      }
    }

    NodeId getN1Id() const { return NIds[0]; }
    NodeId getN2Id() const { return NIds[1]; }

    EdgeMetadata Metadata;

  private:
    NodeId NIds[2];
    AdjEdgeIdx ThisEdgeAdjIdxs[2];
  };

  using NodeVector = std::vector<NodeEntry>;
  using FreeNodeVector = std::vector<NodeId>;
  using EdgeVector = std::vector<EdgeEntry>;
  using FreeEdgeVector = std::vector<EdgeId>;

  NodeVector Nodes;
  FreeNodeVector FreeNodeIds;
  EdgeVector Edges;
  FreeEdgeVector FreeEdgeIds;
  GraphMetadata Metadata;
  SolverTy *Solver = nullptr;

  NodeEntry &getNode(NodeId NId) {
    assert(NId < Nodes.size() && "Out of bound NodeId");
    return Nodes[NId];
  }
  const NodeEntry &getNode(NodeId NId) const {
    assert(NId < Nodes.size() && "Out of bound NodeId");
    return Nodes[NId];
  }

  EdgeEntry &getEdge(EdgeId EId) {
    assert(EId < Edges.size() && "Out of bound EdgeId");
    return Edges[EId];
  }
  const EdgeEntry &getEdge(EdgeId EId) const {
    assert(EId < Edges.size() && "Out of bound EdgeId");
    return Edges[EId];
  }

  NodeId addConstructedNode(NodeEntry N) {
    NodeId NId = 0;
    if (!FreeNodeIds.empty()) {
      NId = FreeNodeIds.back();
      FreeNodeIds.pop_back();
      Nodes[NId] = std::move(N);
    } else {
      NId = Nodes.size();
      Nodes.push_back(std::move(N));
    }
    return NId;
  }

  EdgeId addConstructedEdge(EdgeEntry E) {
    assert(findEdge(E.getN1Id(), E.getN2Id()) == invalidEdgeId() &&
           "Attempt to add duplicate edge.");
    EdgeId EId = 0;
    if (!FreeEdgeIds.empty()) {
      EId = FreeEdgeIds.back();
      FreeEdgeIds.pop_back();
      Edges[EId] = std::move(E);
    } else {
      EId = Edges.size();
      Edges.push_back(std::move(E));
    }

    EdgeEntry &NE = getEdge(EId);

    // Add the edge to the adjacency sets of its nodes.
    NE.connect(*this, EId);
    return EId;
  }

public:
  using AdjEdgeItr = typename NodeEntry::AdjEdgeItr;

  class NodeItr {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = NodeId;
    using difference_type = int;
    using pointer = NodeId *;
    using reference = NodeId &;

    NodeItr(NodeId NId, const Graph &G)
        : CurNId(NId), EndNId(G.Nodes.size()), FreeNodeIds(G.FreeNodeIds) {
      CurNId = findNextInUse(NId);
    }

    bool operator==(const NodeItr &O) const { return CurNId == O.CurNId; }
    bool operator!=(const NodeItr &O) const { return !(*this == O); }
    NodeItr &operator++() {
      CurNId = findNextInUse(++CurNId);
      return *this;
    }
    NodeId operator*() const { return CurNId; }

  private:
    NodeId findNextInUse(NodeId NId) const {
      while (NId < EndNId && std::find(FreeNodeIds.begin(), FreeNodeIds.end(),
                                       NId) != FreeNodeIds.end()) {
        ++NId;
      }
      return NId;
    }

    NodeId CurNId, EndNId;
    const FreeNodeVector &FreeNodeIds;
  };

  class EdgeItr {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = EdgeId;
    using difference_type = int;
    using pointer = EdgeId *;
    using reference = EdgeId &;

    EdgeItr(EdgeId CurEId, const Graph &G)
        : CurEId(CurEId), EndEId(G.Edges.size()), FreeEdgeIds(G.FreeEdgeIds) {
      this->CurEId = findNextInUse(CurEId);
    }

    bool operator==(const EdgeItr &O) const { return CurEId == O.CurEId; }
    bool operator!=(const EdgeItr &O) const { return !(*this == O); }
    EdgeItr &operator++() {
      CurEId = findNextInUse(++CurEId);
      return *this;
    }
    EdgeId operator*() const { return CurEId; }

  private:
    EdgeId findNextInUse(EdgeId EId) const {
      while (EId < EndEId && std::find(FreeEdgeIds.begin(), FreeEdgeIds.end(),
                                       EId) != FreeEdgeIds.end()) {
        ++EId;
      }
      return EId;
    }

    EdgeId CurEId, EndEId;
    const FreeEdgeVector &FreeEdgeIds;
  };

  class NodeIdSet {
  public:
    NodeIdSet(const Graph &G) : G(G) {}

    NodeItr begin() const { return NodeItr(0, G); }
    NodeItr end() const { return NodeItr(G.Nodes.size(), G); }

    bool empty() const { return G.Nodes.empty(); }

    typename NodeVector::size_type size() const {
      assert(G.Nodes.size() >= G.FreeNodeIds.size());
      return G.Nodes.size() - G.FreeNodeIds.size();
    }

  private:
    const Graph &G;
  };

  class EdgeIdSet {
  public:
    EdgeIdSet(const Graph &G) : G(G) {}

    EdgeItr begin() const { return EdgeItr(0, G); }
    EdgeItr end() const { return EdgeItr(G.Edges.size(), G); }

    bool empty() const { return G.Edges.empty(); }

    typename EdgeVector::size_type size() const {
      assert(G.Edges.size() >= G.FreeEdgeIds.size());
      return G.Edges.size() - G.FreeEdgeIds.size();
    }

  private:
    const Graph &G;
  };

  class AdjEdgeIdSet {
  public:
    AdjEdgeIdSet(const NodeEntry &NE) : NE(NE) {}

    typename NodeEntry::AdjEdgeItr begin() const {
      return NE.getAdjEdgeIds().begin();
    }

    typename NodeEntry::AdjEdgeItr end() const {
      return NE.getAdjEdgeIds().end();
    }

    bool empty() const { return NE.getAdjEdgeIds().empty(); }

    typename NodeEntry::AdjEdgeList::size_type size() const {
      return NE.getAdjEdgeIds().size();
    }

  private:
    const NodeEntry &NE;
  };

  Graph() = default;

  Graph(GraphMetadata Metadata) : Metadata(std::move(Metadata)) {}

  GraphMetadata& getMetadata() { return Metadata; }

  const GraphMetadata& getMetadata() const { return Metadata; }

  void setSolver(SolverTy &S) {
    assert(!Solver && "Solver already set. Call unsetSolver().");
    Solver = &S;
  }

  void unsetSolver() {
    assert(Solver && "Solver not set.");
    Solver = nullptr;
  }

  template <typename... NodeMetadataArgs>
  NodeId addNode(NodeMetadataArgs &&...MetadataArgs) {
    NodeId NId = addConstructedNode(
        NodeEntry(std::forward<NodeMetadataArgs>(MetadataArgs)...));
    return NId;
  }

  template <typename... EdgeMetadataArgs>
  EdgeId addEdge(NodeId N1Id, NodeId N2Id, EdgeMetadataArgs &&...MetadataArgs) {
    EdgeId EId = addConstructedEdge(
        EdgeEntry(N1Id, N2Id, std::forward<EdgeMetadataArgs>(MetadataArgs)...));
    return EId;
  }

  bool empty() const { return NodeIdSet(*this).empty(); }

  NodeIdSet nodeIds() const { return NodeIdSet(*this); }

  EdgeIdSet edgeIds() const { return EdgeIdSet(*this); }

  AdjEdgeIdSet adjEdgeIds(NodeId NId) const {
    return AdjEdgeIdSet(getNode(NId));
  }

  unsigned getNumNodes() const { return NodeIdSet(*this).size(); }

  unsigned getNumEdges() const { return EdgeIdSet(*this).size(); }

  NodeMetadata &getNodeMetadata(NodeId NId) { return getNode(NId).Metadata; }

  const NodeMetadata &getNodeMetadata(NodeId NId) const { return getNode(NId).Metadata; }

  typename NodeEntry::AdjEdgeList::size_type getNodeDegree(NodeId NId) const {
    return getNode(NId).getAdjEdgeIds().size();
  }

  EdgeMetadata &getEdgeMetadata(EdgeId EId) { return getEdge(EId).Metadata; }

  const EdgeMetadata &getEdgeMetadata(EdgeId EId) const { return getEdge(EId).Metadata; }

  NodeId getEdgeNode1Id(EdgeId EId) const { return getEdge(EId).getN1Id(); }

  NodeId getEdgeNode2Id(EdgeId EId) const { return getEdge(EId).getN2Id(); }

  NodeId getEdgeOtherNodeId(EdgeId EId, NodeId NId) const {
    const EdgeEntry &E = getEdge(EId);
    if (E.getN1Id() == NId) {
      return E.getN2Id();
    }
    if (E.getN2Id() == NId) {
      return E.getN1Id();
    }
    assert(0 && "Invalid other Node Id");
    return invalidNodeId();
  }

  EdgeId findEdge(NodeId N1Id, NodeId N2Id) const {
    auto AdjEIds = adjEdgeIds(N1Id);
    auto Found =
        std::find_if(AdjEIds.begin(), AdjEIds.end(), [&](auto &&AdjEId) {
          return (getEdgeNode1Id(AdjEId) == N2Id) ||
                 (getEdgeNode2Id(AdjEId) == N2Id);
        });
    if (Found != AdjEIds.end()) {
      return *Found;
    }
    return invalidEdgeId();
  }

  void removeNode(NodeId NId) {
    NodeEntry &N = getNode(NId);
    for (AdjEdgeItr AEItr = N.adjEdgesBegin(), AEEnd = N.adjEdgesEnd();
         AEItr != AEEnd;) {
      EdgeId EId = *AEItr;
      ++AEItr;
      removeEdge(EId);
    }
    FreeNodeIds.push_back(NId);
  }

  void removeEdge(EdgeId EId) {
    EdgeEntry &E = getEdge(EId);
    E.disconnectFrom(*this, E.getN1Id());
    E.disconnectFrom(*this, E.getN2Id());
    FreeEdgeIds.push_back(EId);
  }

  void clear() {
    Nodes.clear();
    FreeNodeIds.clear();
    Edges.clear();
    FreeEdgeIds.clear();
  }
};


} // end namespace FICAVCA
} // end namespace llvm

#endif // LLVM_CODEGEN_FICAVCA_GRAPH_H
