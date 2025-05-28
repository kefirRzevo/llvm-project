//===- Graph.h - PBQP Graph -------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// PBQP Graph class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_PBQP_GRAPH_H
#define LLVM_CODEGEN_PBQP_GRAPH_H

#include "llvm/CodeGen/RegAllocGraphAlgorithms/GraphBase.h"
#include "llvm/ADT/STLExtras.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <vector>

namespace llvm {
namespace PBQP {

  /// PBQP Graph class.
  /// Instances of this class describe PBQP problems.
  ///
  template <typename SolverT>
  class Graph : public IGraph<Graph<SolverT>, SolverT> {
    using Base = IGraph<Graph<SolverT>, SolverT>;
    using CostAllocator = typename SolverT::CostAllocator;
    using Base::Solver;
    using Base::getNode;
    using Base::getEdge;
    using Base::addConstructedNode;
    using Base::addConstructedEdge;
    using typename Base::NodeEntry;
    using typename Base::EdgeEntry;

    CostAllocator CostAlloc;

  public:
    using typename Base::NodeId;
    using typename Base::EdgeId;
    using RawVector = typename SolverT::RawVector;
    using RawMatrix = typename SolverT::RawMatrix;
    using Vector = typename SolverT::Vector;
    using Matrix = typename SolverT::Matrix;
    using VectorPtr = typename CostAllocator::VectorPtr;
    using MatrixPtr = typename CostAllocator::MatrixPtr;
    using typename Base::NodeMetadata;
    using typename Base::EdgeMetadata;
    using typename Base::GraphMetadata;

    /// Construct an empty PBQP graph.
    Graph() = default;

    /// Construct an empty PBQP graph with the given graph metadata.
    Graph(GraphMetadata Metadata) : Base(std::move(Metadata)) {}

    /// Add a node with the given costs.
    /// @param Costs Cost vector for the new node.
    /// @return Node iterator for the added node.
    template <typename OtherVectorT>
    NodeId addNode(OtherVectorT Costs) {
      // Get cost vector from the problem domain
      VectorPtr AllocatedCosts = CostAlloc.getVector(std::move(Costs));
      NodeId NId = addConstructedNode(NodeEntry(AllocatedCosts));
      if (Solver)
        Solver->handleAddNode(NId);
      return NId;
    }

    /// Add a node bypassing the cost allocator.
    /// @param Costs Cost vector ptr for the new node (must be convertible to
    ///        VectorPtr).
    /// @return Node iterator for the added node.
    ///
    ///   This method allows for fast addition of a node whose costs don't need
    /// to be passed through the cost allocator. The most common use case for
    /// this is when duplicating costs from an existing node (when using a
    /// pooling allocator). These have already been uniqued, so we can avoid
    /// re-constructing and re-uniquing them by attaching them directly to the
    /// new node.
    template <typename OtherVectorPtrT>
    NodeId addNodeBypassingCostAllocator(OtherVectorPtrT Costs) {
      NodeId NId = addConstructedNode(NodeEntry(Costs));
      if (Solver)
        Solver->handleAddNode(NId);
      return NId;
    }

    /// Add an edge between the given nodes with the given costs.
    /// @param N1Id First node.
    /// @param N2Id Second node.
    /// @param Costs Cost matrix for new edge.
    /// @return Edge iterator for the added edge.
    template <typename OtherVectorT>
    EdgeId addEdge(NodeId N1Id, NodeId N2Id, OtherVectorT Costs) {
      assert(getNodeCosts(N1Id).getLength() == Costs.getRows() &&
             getNodeCosts(N2Id).getLength() == Costs.getCols() &&
             "Matrix dimensions mismatch.");
      // Get cost matrix from the problem domain.
      MatrixPtr AllocatedCosts = CostAlloc.getMatrix(std::move(Costs));
      EdgeId EId = addConstructedEdge(EdgeEntry(N1Id, N2Id, AllocatedCosts));
      if (Solver)
        Solver->handleAddEdge(EId);
      return EId;
    }

    /// Add an edge bypassing the cost allocator.
    /// @param N1Id First node.
    /// @param N2Id Second node.
    /// @param Costs Cost matrix for new edge.
    /// @return Edge iterator for the added edge.
    ///
    ///   This method allows for fast addition of an edge whose costs don't need
    /// to be passed through the cost allocator. The most common use case for
    /// this is when duplicating costs from an existing edge (when using a
    /// pooling allocator). These have already been uniqued, so we can avoid
    /// re-constructing and re-uniquing them by attaching them directly to the
    /// new edge.
    template <typename OtherMatrixPtrT>
    NodeId addEdgeBypassingCostAllocator(NodeId N1Id, NodeId N2Id,
                                         OtherMatrixPtrT Costs) {
      assert(getNodeCosts(N1Id).getLength() == Costs->getRows() &&
             getNodeCosts(N2Id).getLength() == Costs->getCols() &&
             "Matrix dimensions mismatch.");
      // Get cost matrix from the problem domain.
      EdgeId EId = addConstructedEdge(EdgeEntry(N1Id, N2Id, Costs));
      if (Solver)
        Solver->handleAddEdge(EId);
      return EId;
    }

    /// Set a node's cost vector.
    /// @param NId Node to update.
    /// @param Costs New costs to set.
    template <typename OtherVectorT>
    void setNodeCosts(NodeId NId, OtherVectorT Costs) {
      VectorPtr AllocatedCosts = CostAlloc.getVector(std::move(Costs));
      if (Solver)
        Solver->handleSetNodeCosts(NId, *AllocatedCosts);
      getNode(NId).Data = AllocatedCosts;
    }

    /// Get a VectorPtr to a node's cost vector. Rarely useful - use
    ///        getNodeCosts where possible.
    /// @param NId Node id.
    /// @return VectorPtr to node cost vector.
    ///
    ///   This method is primarily useful for duplicating costs quickly by
    /// bypassing the cost allocator. See addNodeBypassingCostAllocator. Prefer
    /// getNodeCosts when dealing with node cost values.
    const VectorPtr& getNodeCostsPtr(NodeId NId) const {
      return getNode(NId).Data;
    }

    /// Get a node's cost vector.
    /// @param NId Node id.
    /// @return Node cost vector.
    const Vector& getNodeCosts(NodeId NId) const {
      return *getNodeCostsPtr(NId);
    }

    /// Update an edge's cost matrix.
    /// @param EId Edge id.
    /// @param Costs New cost matrix.
    template <typename OtherMatrixT>
    void updateEdgeCosts(EdgeId EId, OtherMatrixT Costs) {
      MatrixPtr AllocatedCosts = CostAlloc.getMatrix(std::move(Costs));
      if (Solver)
        Solver->handleUpdateCosts(EId, *AllocatedCosts);
      getEdge(EId).Data = AllocatedCosts;
    }

    /// Get a MatrixPtr to a node's cost matrix. Rarely useful - use
    ///        getEdgeCosts where possible.
    /// @param EId Edge id.
    /// @return MatrixPtr to edge cost matrix.
    ///
    ///   This method is primarily useful for duplicating costs quickly by
    /// bypassing the cost allocator. See addNodeBypassingCostAllocator. Prefer
    /// getEdgeCosts when dealing with edge cost values.
    const MatrixPtr& getEdgeCostsPtr(EdgeId EId) const {
      return getEdge(EId).Data;
    }

    /// Get an edge's cost matrix.
    /// @param EId Edge id.
    /// @return Edge cost matrix.
    const Matrix& getEdgeCosts(EdgeId EId) const {
      return *getEdge(EId).Data;
    }
  };

} // end namespace PBQP
} // end namespace llvm

#endif // LLVM_CODEGEN_PBQP_GRAPH_H
