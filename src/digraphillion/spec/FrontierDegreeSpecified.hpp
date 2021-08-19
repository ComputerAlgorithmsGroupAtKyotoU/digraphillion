#ifndef FRONTIER_DEGREE_SPECIFIED_HPP
#define FRONTIER_DEGREE_SPECIFIED_HPP

#include <climits>
#include <vector>

#include "FrontierData.hpp"
#include "subsetting/util/IntSubset.hpp"

using namespace tdzdd;

typedef unsigned char uchar;

class FrontierDegreeSpecifiedSpec
    : public tdzdd::PodArrayDdSpec<FrontierDegreeSpecifiedSpec, DirectedFrontierData,
                                   2> {
 private:
  // input graph
  const tdzdd::Graph& graph_;
  // number of vertices
  const int n_;
  // number of edges
  const int m_;

  const FrontierManager fm_;

  const std::vector<IntSubset*> degRanges_;

  // This function gets deg of v.
  int getDeg(FrontierDSData* data, int v) const {
    return static_cast<int>(data[fm_.vertexToPos(v) * 2]);
  }

  // This function sets deg of v to be d.
  void setDeg(FrontierDSData* data, int v, int d) const {
    data[fm_.vertexToPos(v) * 2] = static_cast<uchar>(d);
  }

  int getDegUpper(FrontierDSData* data) const {
    int deg;
    for (deg = static_cast<int>(degRanges_.size()) - 1; deg >= 0; --deg) {
      if (data[fixedDegStart_ + deg] < degRanges_[deg]->upperBound()) {
        break;
      }
    }
    return deg;
  }

  void initializeData(FrontierDSData* data) const {
    for (int i = 0; i < fixedDegStart_ + static_cast<int>(degRanges_.size());
         ++i) {
      data[i] = 0;
    }
  }

 public:
  FrontierDegreeSpecifiedSpec(const tdzdd::Graph& graph,
                              const std::vector<IntSubset*>& degRanges)
      : graph_(graph),
        n_(static_cast<short>(graph_.vertexSize())),
        m_(graph_.edgeSize()),
        fm_(graph_),
        fixedDegStart_(fm_.getMaxFrontierSize() * 2),
        degRanges_(degRanges) {
    if (graph_.vertexSize() > SHRT_MAX) {  // SHRT_MAX == 32767
      std::cerr << "The number of vertices should be at most " << SHRT_MAX
                << std::endl;
      exit(1);
    }

    // todo: check all the degrees is at most 256

    setArraySize(fixedDegStart_ + degRanges_.size());
  }

  int getRoot(FrontierDSData* data) const {
    initializeData(data);
    return m_;
  }

  int getChild(FrontierDSData* data, int level, int value) const {
    assert(1 <= level && level <= m_);

    // std::cerr << "level = " << level << ", value = " << value << std::endl;

    // edge index (starting from 0)
    const int edge_index = m_ - level;
    // edge that we are processing.
    // The endpoints of "edge" are edge.v1 and edge.v2.
    const Graph::EdgeInfo& edge = graph_.edgeInfo(edge_index);

    // initialize deg and comp of the vertices newly entering the frontier
    const std::vector<int>& entering_vs = fm_.getEnteringVs(edge_index);
    for (size_t i = 0; i < entering_vs.size(); ++i) {
      int v = entering_vs[i];
      // initially the value of deg is 0
      setDeg(data, v, 0);
    }

    // vertices on the frontier
    const std::vector<int>& frontier_vs = fm_.getFrontierVs(edge_index);

    if (value == 1) {  // if we take the edge (go to 1-arc)
      // increment deg of v1 and v2 (recall that edge = {v1, v2})

      int upper = getDegUpper(data);
      if (getDeg(data, edge.v1) + 1 > upper) {
        return 0;
      }
      if (getDeg(data, edge.v2) + 1 > upper) {
        return 0;
      }
      setDeg(data, edge.v1, getDeg(data, edge.v1) + 1);
      setDeg(data, edge.v2, getDeg(data, edge.v2) + 1);
    }

    // vertices that are leaving the frontier
    const std::vector<int>& leaving_vs = fm_.getLeavingVs(edge_index);
    for (size_t i = 0; i < leaving_vs.size(); ++i) {
      int v = leaving_vs[i];

      int d = getDeg(data, v);

      bool samecomp_found = false;
      bool nonisolated_found = false;

      // Search a vertex that has the component number same as that of v.
      // Also check whether a vertex whose degree is at least 1 exists
      // on the frontier.
      for (size_t j = 0; j < frontier_vs.size(); ++j) {
        int w = frontier_vs[j];
        if (w == v) {  // skip if w is the leaving vertex
          continue;
        }
        // skip if w is one of the vertices that
        // has already leaved the frontier
        bool found_leaved = false;
        for (size_t k = 0; k < i; ++k) {
          if (w == leaving_vs[k]) {
            found_leaved = true;
            break;
          }
        }
        if (found_leaved) {
          continue;
        }
        // The degree of w is at least 1.
        if (getDeg(data, w) > 0) {
          nonisolated_found = true;
        }
        if (nonisolated_found && samecomp_found) {
          break;
        }
      }
      // There is no vertex that has the component number
      // same as that of v. That is, the connected component
      // of v becomes determined.
      if (!samecomp_found) {
        // Check whether v is isolated.
        // If v is isolated (deg of v is 0), nothing occurs.
        if (d > 0) {
          // Check whether there is a
          // connected component other than that of v,
          // that is, the generated subgraph is not connected.
          // If so, we return the 0-terminal.
          if (nonisolated_found) {
            return 0;  // return the 0-terminal.
          } else {
            if (checkFixedDeg(data)) {
              return -1;
            } else {
              return 0;
            }
          }
        }
      }
      // Since deg of v is never used until the end,
      // we erase the values.
      setDeg(data, v, -1);
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
