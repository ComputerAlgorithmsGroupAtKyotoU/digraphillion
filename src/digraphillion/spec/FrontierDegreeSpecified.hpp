#ifndef FRONTIER_DEGREE_SPECIFIED_HPP
#define FRONTIER_DEGREE_SPECIFIED_HPP

#include <climits>
#include <vector>

#include "FrontierData.hpp"
#include "FrontierManager.hpp"
#include "subsetting/DdSpec.hpp"
#include "subsetting/util/Digraph.hpp"
#include "subsetting/util/IntSubset.hpp"

using namespace tdzdd;

class FrontierDegreeSpecifiedSpec
    : public tdzdd::PodArrayDdSpec<FrontierDegreeSpecifiedSpec,
                                   DirectedFrontierData, 2> {
 private:
  // input graph
  const tdzdd::Digraph& graph_;
  // number of vertices
  const int n_;
  // number of edges
  const int m_;

  const FrontierManager fm_;

  std::vector<IntSubset const*> in_constraints;
  std::vector<IntSubset const*> out_constraints;

  // This function gets deg of v.
  short getIndeg(DirectedFrontierData* data, short v) const {
    return data[fm_.vertexToPos(v)].indeg;
  }

  short getOutdeg(DirectedFrontierData* data, short v) const {
    return data[fm_.vertexToPos(v)].outdeg;
  }

  // This function sets deg of v to be d.
  void setIndeg(DirectedFrontierData* data, short v, short d) const {
    data[fm_.vertexToPos(v)].indeg = d;
  }

  void setOutdeg(DirectedFrontierData* data, short v, short d) const {
    data[fm_.vertexToPos(v)].outdeg = d;
  }

  void initializeData(DirectedFrontierData* data) const {
    for (int i = 0; i < fm_.getMaxFrontierSize(); ++i) {
      data[i].indeg = 0;
      data[i].outdeg = 0;
      data[i].comp = 0;
    }
  }

 public:
  FrontierDegreeSpecifiedSpec(const tdzdd::Digraph& graph,
                              IntSubset const* c = 0)
      : graph_(graph),
        n_(static_cast<short>(graph_.vertexSize())),
        m_(graph_.edgeSize()),
        fm_(graph_) {
    if (graph_.vertexSize() > SHRT_MAX) {  // SHRT_MAX == 32767
      std::cerr << "The number of vertices should be at most " << SHRT_MAX
                << std::endl;
      exit(1);
    }

    // todo: check all the degrees is at most 256

    setArraySize(fm_.getMaxFrontierSize());
    int m = graph_.vertexSize();
    in_constraints.resize(m + 1);
    out_constraints.resize(m + 1);
    for (int v = 1; v <= m; v++) {
      in_constraints[v] = c;
      out_constraints[v] = c;
    }
  }

  void setIndegConstraint(Digraph::VertexNumber v, IntSubset const* c) {
    if (v < 1 || graph_.vertexSize() < v)
      throw std::runtime_error("ERROR: Vertex number is out of range");
    in_constraints[v] = c;
  }

  void setOutdegConstraint(Digraph::VertexNumber v, IntSubset const* c) {
    if (v < 1 || graph_.vertexSize() < v)
      throw std::runtime_error("ERROR: Vertex number is out of range");
    out_constraints[v] = c;
  }

  int getRoot(DirectedFrontierData* data) const {
    initializeData(data);
    return m_;
  }

  int getChild(DirectedFrontierData* data, int level, int value) const {
    assert(1 <= level && level <= m_);

    // edge index (starting from 0)
    const int edge_index = m_ - level;
    // edge that we are processing.
    // The endpoints of "edge" are edge.v1 and edge.v2.
    const Digraph::EdgeInfo& edge = graph_.edgeInfo(edge_index);

    // initialize deg of the vertices newly entering the frontier
    const std::vector<int>& entering_vs = fm_.getEnteringVs(edge_index);
    for (size_t i = 0; i < entering_vs.size(); ++i) {
      int v = entering_vs[i];
      // initially the value of deg is 0
      setIndeg(data, v, 0);
      setOutdeg(data, v, 0);
    }

    if (value == 1) {  // if we take the edge (go to 1-arc)
      // increment deg of v1 and v2 (recall that edge = {v1, v2})
      auto outdeg1 = getOutdeg(data, edge.v1);
      assert(1 <= edge.v1 && edge.v1 < (int)out_constraints.size());
      if (!out_constraints[edge.v1]->contains(outdeg1 + 1)) {
        return 0;
      }

      auto indeg2 = getIndeg(data, edge.v2);
      assert(1 <= edge.v2 && edge.v2 < (int)in_constraints.size());
      if (!in_constraints[edge.v2]->contains(indeg2 + 1)) {
        return 0;
      }
      setIndeg(data, edge.v2, indeg2 + 1);
      setOutdeg(data, edge.v1, outdeg1 + 1);
    }

    // vertices that are leaving the frontier
    const std::vector<int>& leaving_vs = fm_.getLeavingVs(edge_index);
    for (size_t i = 0; i < leaving_vs.size(); ++i) {
      int v = leaving_vs[i];

      int indeg = getIndeg(data, v), outdeg = getOutdeg(data, v);
      assert(1 <= v && v < (int)in_constraints.size());
      assert(1 <= v && v < (int)out_constraints.size());
      if (!in_constraints[v]->contains(indeg) ||
          !out_constraints[v]->contains(outdeg)) {
        return 0;
      }

      // Since deg of v is never used until the end,
      // we erase the values.
      setIndeg(data, v, -1);
      setOutdeg(data, v, -1);
    }
    if (level == 1) {
      // If we come here, the edge set is empty (taking no edge).
      return 0;
    }
    assert(level - 1 > 0);
    return level - 1;
  }
};

#endif  // FRONTIER_DEGREE_SPECIFIED_HPP
