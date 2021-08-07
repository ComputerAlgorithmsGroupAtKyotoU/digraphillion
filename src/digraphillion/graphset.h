#ifndef DIGRAPHILLION_GRAPHSET_H_
#define DIGRAPHILLION_GRAPHSET_H_

#include "digraphillion/setset.h"
#include "subsetting/util/IntSubset.hpp"

namespace digraphillion {

class Range : public tdzdd::IntSubset {
 public:
  Range(int max = 1);
  Range(int min, int max, int step = 1);

  bool contains(int x) const;
  int lowerBound() const;
  int upperBound() const;

 private:
  int min_;
  int max_;
  int step_;
};

setset SearchDirectedCycles(const std::vector<edge_t>& digraph,
                            const setset* search_space);

setset SearchDirectedHamiltonianCycles(const std::vector<edge_t>& digraph,
                                       const setset* search_space);

setset SearchDirectedSTPath(const std::vector<edge_t>& digraph,
                            bool is_hamiltonian, vertex_t s, vertex_t t,
                            const setset* search_space);

setset SearchDirectedForests(const std::vector<edge_t>& digraph,
                             const setset* search_space);

setset SearchRootedTrees(const std::vector<edge_t>& digraph, vertex_t root,
                         bool is_spanning, const setset* search_space);

bool ShowMessages(bool flag = true);

}  // namespace digraphillion

#endif  // DIGRAPHILLION_GRAPHSET_H_
