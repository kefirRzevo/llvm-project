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
#include "llvm/CodeGen/RegAllocGraphAlgorithms/GraphBase.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <vector>

namespace llvm {
namespace FICAVCA {

template <typename SolverT>
class Graph : public IGraph<Graph<SolverT>, SolverT> {
  using Base = IGraph<Graph<SolverT>, SolverT>;
  using Base::Solver;
  using Base::getNode;
  using Base::getEdge;
  using Base::addConstructedNode;
  using Base::addConstructedEdge;
  using typename Base::NodeEntry;
  using typename Base::EdgeEntry;

public:
  using typename Base::NodeId;
  using typename Base::EdgeId;
  using typename Base::NodeData;
  using typename Base::NodeMetadata;
  using typename Base::EdgeData;
  using typename Base::EdgeMetadata;
  using typename Base::GraphMetadata;

public:
  Graph() = default;

  Graph(GraphMetadata Metadata) : Base(std::move(Metadata)) {}

  template <typename... NodeDataArgs>
  NodeId addNode(NodeDataArgs &&...DataArgs) {
    NodeData Data(std::forward<NodeDataArgs>(DataArgs)...);
    NodeId NId = addConstructedNode(NodeEntry(std::move(Data)));
    if (Solver)
      Solver->handleAddNode(NId);
    return NId;
  }

  template <typename... EdgeDataArgs>
  EdgeId addEdge(NodeId N1Id, NodeId N2Id, EdgeDataArgs &&...DataArgs) {
    EdgeMetadata Data(std::forward<EdgeDataArgs>(DataArgs)...);
    EdgeId EId = addConstructedEdge(EdgeEntry(N1Id, N2Id, std::move(Data)));
    if (Solver)
      Solver->handleAddEdge(EId);
    return EId;
    }

};


} // end namespace FICAVCA
} // end namespace llvm

#endif // LLVM_CODEGEN_FICAVCA_GRAPH_H
