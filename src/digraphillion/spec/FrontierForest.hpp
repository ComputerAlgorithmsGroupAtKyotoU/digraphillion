#ifndef FRONTIER_FOREST_HPP
#define FRONTIER_FOREST_HPP

#include <climits>
#include <vector>

#include "FrontierData.hpp"
#include "FrontierManager.hpp"
#include "subsetting/DdSpec.hpp"
#include "subsetting/util/Digraph.hpp"

using namespace tdzdd;

typedef unsigned short ushort;

class FrontierDirectedForestSpec
    : public tdzdd::PodArrayDdSpec<FrontierDirectedForestSpec,
                                   DirectedFrontierData, 2> {
 private:
  // input graph
  const tdzdd::Digraph& graph_;
  // number of vertices
  const short n_;
  // number of edges
  const int m_;

  const FrontierManager fm_;

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

  // This function gets comp of v.
  ushort getComp(DirectedFrontierData* data, short v) const {
    return data[fm_.vertexToPos(v)].comp;
  }

  // This function sets comp of v to be c.
  void setComp(DirectedFrontierData* data, short v, ushort c) const {
    data[fm_.vertexToPos(v)].comp = c;
  }

  void initializeData(DirectedFrontierData* data) const {
    for (int i = 0; i < fm_.getMaxFrontierSize(); ++i) {
      data[i].indeg = 0;
      data[i].outdeg = 0;
      data[i].comp = 0;
    }
  }

 public:
  FrontierDirectedForestSpec(const tdzdd::Digraph& graph)
      : graph_(graph),
        n_(static_cast<short>(graph_.vertexSize())),
        m_(graph_.edgeSize()),
        fm_(graph_) {
    if (n_ >= (1 << 16)) {
      std::cerr << "The number of vertices must be smaller than 2^15."
                << std::endl;
      exit(1);
    }
    setArraySize(fm_.getMaxFrontierSize());
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

    // initialize deg and comp of the vertices newly entering the frontier
    const std::vector<int>& entering_vs = fm_.getEnteringVs(edge_index);
    for (size_t i = 0; i < entering_vs.size(); ++i) {
      int v = entering_vs[i];
      // initially the value of deg is 0
      setIndeg(data, v, 0);
      setOutdeg(data, v, 0);
      // initially the value of comp is the vertex number itself
      setComp(data, v, static_cast<ushort>(v));
    }

    // vertices on the frontier
    const std::vector<int>& frontier_vs = fm_.getFrontierVs(edge_index);

    if (value == 1) {  // if we take the edge (go to 1-arc)
      // increment deg of v1 and v2 (recall that edge = {v1, v2})
      auto outdeg1 = getOutdeg(data, edge.v1);
      auto indeg2 = getIndeg(data, edge.v2);

      setIndeg(data, edge.v2, indeg2 + 1);
      setOutdeg(data, edge.v1, outdeg1 + 1);

      ushort c1 = getComp(data, edge.v1);
      ushort c2 = getComp(data, edge.v2);

      if (c1 == c2) {  // Any cycle must not occur.
        return 0;
      }

      if (c1 != c2) {  // connected components c1 and c2 become connected
        ushort cmin = std::min(c1, c2);
        ushort cmax = std::max(c1, c2);

        // replace component number cmin with cmax
        for (size_t i = 0; i < frontier_vs.size(); ++i) {
          int v = frontier_vs[i];
          if (getComp(data, v) == cmin) {
            setComp(data, v, cmax);
          }
        }
      }
    }

    // vertices that are leaving the frontier
    const std::vector<int>& leaving_vs = fm_.getLeavingVs(edge_index);
    for (size_t i = 0; i < leaving_vs.size(); ++i) {
      int v = leaving_vs[i];

      // the in-degree of v must be 0 or 1.
      if (getIndeg(data, v) > 1) {
        return 0;
      }

      // Since deg and comp of v are never used until the end,
      // we erase the values.
      setIndeg(data, v, 0);
      setOutdeg(data, v, 0);
      setComp(data, v, 0);
    }
    if (level == 1) {
      return -1;
    }
    assert(level - 1 > 0);
    return level - 1;
  }
};

#endif  // FRONTIER_FOREST_HPP
