//===- Graph.h - FICAVCA Graph ----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Base graph class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_GRAPH_BASE_H
#define LLVM_CODEGEN_GRAPH_BASE_H

#include "llvm/ADT/STLExtras.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <vector>

namespace llvm {

/// Base graph class for RegAlloc graph algorithms.
///
class GraphBase {
public:
  using NodeId = unsigned;
  using EdgeId = unsigned;

  /// Returns a value representing an invalid (non-existent) node.
  static NodeId invalidNodeId() { return std::numeric_limits<NodeId>::max(); }

  /// Returns a value representing an invalid (non-existent) edge.
  static EdgeId invalidEdgeId() { return std::numeric_limits<EdgeId>::max(); }
};

/// Base graph class for RegAlloc graph algorithms.
///
template <typename Graph, typename SolverT> class IGraph : public GraphBase {
public:
  using NodeData = typename SolverT::NodeData;
  using NodeMetadata = typename SolverT::NodeMetadata;
  using EdgeData = typename SolverT::EdgeData;
  using EdgeMetadata = typename SolverT::EdgeMetadata;
  using GraphMetadata = typename SolverT::GraphMetadata;

private:
  Graph &impl() { return static_cast<Graph &>(*this); }

  const Graph &impl() const { return static_cast<const Graph &>(*this); }

protected:
  class NodeEntry {
  public:
    using AdjEdgeList = std::vector<EdgeId>;
    using AdjEdgeIdx = AdjEdgeList::size_type;
    using AdjEdgeItr = AdjEdgeList::const_iterator;

    NodeEntry(NodeData Data) : Data(std::move(Data)) {}

    static AdjEdgeIdx getInvalidAdjEdgeIdx() {
      return std::numeric_limits<AdjEdgeIdx>::max();
    }

    AdjEdgeIdx addAdjEdgeId(EdgeId EId) {
      AdjEdgeIdx Idx = AdjEdgeIds.size();
      AdjEdgeIds.push_back(EId);
      return Idx;
    }

    void removeAdjEdgeId(Graph &G, NodeId ThisNId, AdjEdgeIdx Idx) {
      // Swap-and-pop for fast removal.
      //   1) Update the adj index of the edge currently at back().
      //   2) Move last Edge down to Idx.
      //   3) pop_back()
      // If Idx == size() - 1 then the setAdjEdgeIdx and swap are
      // redundant, but both operations are cheap.
      static_cast<IGraph &>(G)
          .getEdge(AdjEdgeIds.back())
          .setAdjEdgeIdx(ThisNId, Idx);
      AdjEdgeIds[Idx] = AdjEdgeIds.back();
      AdjEdgeIds.pop_back();
    }

    const AdjEdgeList &getAdjEdgeIds() const { return AdjEdgeIds; }

    NodeData Data;
    NodeMetadata Metadata;

  private:
    AdjEdgeList AdjEdgeIds;
  };

  class EdgeEntry {
  public:
    EdgeEntry(NodeId N1Id, NodeId N2Id, EdgeData Data) : Data(std::move(Data)) {
      NIds[0] = N1Id;
      NIds[1] = N2Id;
      ThisEdgeAdjIdxs[0] = NodeEntry::getInvalidAdjEdgeIdx();
      ThisEdgeAdjIdxs[1] = NodeEntry::getInvalidAdjEdgeIdx();
    }

    void connectToN(Graph &G, EdgeId ThisEdgeId, unsigned NIdx) {
      assert(ThisEdgeAdjIdxs[NIdx] == NodeEntry::getInvalidAdjEdgeIdx() &&
             "Edge already connected to NIds[NIdx].");
      NodeEntry &N = static_cast<IGraph &>(G).getNode(NIds[NIdx]);
      ThisEdgeAdjIdxs[NIdx] = N.addAdjEdgeId(ThisEdgeId);
    }

    void connect(Graph &G, EdgeId ThisEdgeId) {
      connectToN(G, ThisEdgeId, 0);
      connectToN(G, ThisEdgeId, 1);
    }

    void setAdjEdgeIdx(NodeId NId, typename NodeEntry::AdjEdgeIdx NewIdx) {
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
      NodeEntry &N = static_cast<IGraph &>(G).getNode(NIds[NIdx]);
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

    EdgeData Data;
    EdgeMetadata Metadata;

  private:
    NodeId NIds[2];
    typename NodeEntry::AdjEdgeIdx ThisEdgeAdjIdxs[2];
  };
  // ----- MEMBERS -----

protected:
  SolverT *Solver = nullptr;

  // ----- INTERNAL METHODS -----

  NodeEntry &getNode(NodeId NId) {
    assert(NId < Nodes.size() && "Out of bound NodeId");
    return Nodes[NId];
  }
  const NodeEntry &getNode(NodeId NId) const {
    assert(NId < Nodes.size() && "Out of bound NodeId");
    return Nodes[NId];
  }

  EdgeEntry &getEdge(EdgeId EId) { return Edges[EId]; }
  const EdgeEntry &getEdge(EdgeId EId) const { return Edges[EId]; }

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
    NE.connect(impl(), EId);
    return EId;
  }

private:
  GraphMetadata Metadata;

  using NodeVector = std::vector<NodeEntry>;
  using FreeNodeVector = std::vector<NodeId>;
  NodeVector Nodes;
  FreeNodeVector FreeNodeIds;

  using EdgeVector = std::vector<EdgeEntry>;
  using FreeEdgeVector = std::vector<EdgeId>;
  EdgeVector Edges;
  FreeEdgeVector FreeEdgeIds;

public:
  using AdjEdgeItr = typename NodeEntry::AdjEdgeItr;

  class NodeItr {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = NodeId;
    using difference_type = int;
    using pointer = NodeId *;
    using reference = NodeId &;

    NodeItr(NodeId CurNId, const Graph &G)
        : CurNId(CurNId), EndNId(G.Nodes.size()), FreeNodeIds(G.FreeNodeIds) {
      this->CurNId = findNextInUse(CurNId); // Move to first in-use node id
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
      while (NId < EndNId && is_contained(FreeNodeIds, NId)) {
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
      this->CurEId = findNextInUse(CurEId); // Move to first in-use edge id
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
      while (EId < EndEId && is_contained(FreeEdgeIds, EId)) {
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

    typename NodeVector::size_type size() const {
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

  /// Construct an empty Graph.
  IGraph() = default;

  /// Construct an empty PBQP graph with the given graph metadata.
  IGraph(GraphMetadata Data) : Metadata(std::move(Data)) {}

  /// Get a reference to the graph metadata.
  GraphMetadata &getMetadata() { return Metadata; }

  /// Get a const-reference to the graph metadata.
  const GraphMetadata &getMetadata() const { return Metadata; }

  /// Lock this graph to the given solver instance in preparation
  /// for running the solver. This method will call solver.handleAddNode for
  /// each node in the graph, and handleAddEdge for each edge, to give the
  /// solver an opportunity to set up any requried metadata.
  void setSolver(SolverT &S) {
    assert(!Solver && "Solver already set. Call unsetSolver().");
    Solver = &S;
    for (auto NId : nodeIds())
      Solver->handleAddNode(NId);
    for (auto EId : edgeIds())
      Solver->handleAddEdge(EId);
  }

  /// Release from solver instance.
  void unsetSolver() {
    assert(Solver && "Solver not set.");
    Solver = nullptr;
  }

  /// Returns true if the graph is empty.
  bool empty() const { return NodeIdSet(impl()).empty(); }

  NodeIdSet nodeIds() const { return NodeIdSet(impl()); }
  EdgeIdSet edgeIds() const { return EdgeIdSet(impl()); }

  AdjEdgeIdSet adjEdgeIds(NodeId NId) const {
    return AdjEdgeIdSet(getNode(NId));
  }

  /// Get the number of nodes in the graph.
  /// @return Number of nodes in the graph.
  unsigned getNumNodes() const { return NodeIdSet(impl()).size(); }

  /// Get the number of edges in the graph.
  /// @return Number of edges in the graph.
  unsigned getNumEdges() const { return EdgeIdSet(impl()).size(); }

  NodeMetadata &getNodeMetadata(NodeId NId) { return getNode(NId).Metadata; }

  const NodeMetadata &getNodeMetadata(NodeId NId) const {
    return getNode(NId).Metadata;
  }

  typename NodeEntry::AdjEdgeList::size_type getNodeDegree(NodeId NId) const {
    return getNode(NId).getAdjEdgeIds().size();
  }

  EdgeMetadata &getEdgeMetadata(EdgeId EId) { return getEdge(EId).Metadata; }

  const EdgeMetadata &getEdgeMetadata(EdgeId EId) const {
    return getEdge(EId).Metadata;
  }

  /// Get the first node connected to this edge.
  /// @param EId Edge id.
  /// @return The first node connected to the given edge.
  NodeId getEdgeNode1Id(EdgeId EId) const { return getEdge(EId).getN1Id(); }

  /// Get the second node connected to this edge.
  /// @param EId Edge id.
  /// @return The second node connected to the given edge.
  NodeId getEdgeNode2Id(EdgeId EId) const { return getEdge(EId).getN2Id(); }

  /// Get the "other" node connected to this edge.
  /// @param EId Edge id.
  /// @param NId Node id for the "given" node.
  /// @return The iterator for the "other" node connected to this edge.
  NodeId getEdgeOtherNodeId(EdgeId EId, NodeId NId) {
    EdgeEntry &E = getEdge(EId);
    if (E.getN1Id() == NId) {
      return E.getN2Id();
    } // else
    return E.getN1Id();
  }

  /// Get the edge connecting two nodes.
  /// @param N1Id First node id.
  /// @param N2Id Second node id.
  /// @return An id for edge (N1Id, N2Id) if such an edge exists,
  ///         otherwise returns an invalid edge id.
  EdgeId findEdge(NodeId N1Id, NodeId N2Id) {
    for (auto AEId : adjEdgeIds(N1Id)) {
      if ((getEdgeNode1Id(AEId) == N2Id) || (getEdgeNode2Id(AEId) == N2Id)) {
        return AEId;
      }
    }
    return invalidEdgeId();
  }

  /// Remove a node from the graph.
  /// @param NId Node id.
  void removeNode(NodeId NId) {
    if (Solver)
      Solver->handleRemoveNode(NId);
    NodeEntry &N = getNode(NId);
    // TODO: Can this be for-each'd?
    for (AdjEdgeItr AEItr = N.adjEdgesBegin(), AEEnd = N.adjEdgesEnd();
         AEItr != AEEnd;) {
      EdgeId EId = *AEItr;
      ++AEItr;
      removeEdge(EId);
    }
    FreeNodeIds.push_back(NId);
  }

  /// Disconnect an edge from the given node.
  ///
  /// Removes the given edge from the adjacency list of the given node.
  /// This operation leaves the edge in an 'asymmetric' state: It will no
  /// longer appear in an iteration over the given node's (NId's) edges, but
  /// will appear in an iteration over the 'other', unnamed node's edges.
  ///
  /// This does not correspond to any normal graph operation, but exists to
  /// support efficient PBQP graph-reduction based solvers. It is used to
  /// 'effectively' remove the unnamed node from the graph while the solver
  /// is performing the reduction. The solver will later call reconnectNode
  /// to restore the edge in the named node's adjacency list.
  ///
  /// Since the degree of a node is the number of connected edges,
  /// disconnecting an edge from a node 'u' will cause the degree of 'u' to
  /// drop by 1.
  ///
  /// A disconnected edge WILL still appear in an iteration over the graph
  /// edges.
  ///
  /// A disconnected edge should not be removed from the graph, it should be
  /// reconnected first.
  ///
  /// A disconnected edge can be reconnected by calling the reconnectEdge
  /// method.
  void disconnectEdge(EdgeId EId, NodeId NId) {
    if (Solver)
      Solver->handleDisconnectEdge(EId, NId);

    EdgeEntry &E = getEdge(EId);
    E.disconnectFrom(impl(), NId);
  }

  /// Convenience method to disconnect all neighbours from the given
  ///        node.
  void disconnectAllNeighborsFromNode(NodeId NId) {
    for (auto AEId : adjEdgeIds(NId))
      disconnectEdge(AEId, getEdgeOtherNodeId(AEId, NId));
  }

  /// Re-attach an edge to its nodes.
  ///
  /// Adds an edge that had been previously disconnected back into the
  /// adjacency set of the nodes that the edge connects.
  void reconnectEdge(EdgeId EId, NodeId NId) {
    EdgeEntry &E = getEdge(EId);
    E.connectTo(impl(), EId, NId);
    if (Solver)
      Solver->handleReconnectEdge(EId, NId);
  }

  /// Remove an edge from the graph.
  /// @param EId Edge id.
  void removeEdge(EdgeId EId) {
    if (Solver)
      Solver->handleRemoveEdge(EId);
    EdgeEntry &E = getEdge(EId);
    E.disconnect();
    FreeEdgeIds.push_back(EId);
    Edges[EId].invalidate();
  }

  /// Remove all nodes and edges from the graph.
  void clear() {
    Nodes.clear();
    FreeNodeIds.clear();
    Edges.clear();
    FreeEdgeIds.clear();
  }
};

} // end namespace llvm

#endif // LLVM_CODEGEN_GRAPH_BASE_H
