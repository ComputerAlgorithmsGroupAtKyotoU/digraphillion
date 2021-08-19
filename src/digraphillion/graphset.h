#ifndef DIGRAPHILLION_GRAPHSET_H_
#define DIGRAPHILLION_GRAPHSET_H_

#include "digraphillion/setset.h"
#include "subsetting/util/IntRange.hpp"

namespace digraphillion {
setset SearchDirectedCycles(const std::vector<edge_t>& digraph,
                            const setset* search_space);

setset SearchDirectedHamiltonianCycles(const std::vector<edge_t>& digraph,
                                       const setset* search_space);

setset SearchDirectedSTPath(const std::vector<edge_t>& digraph,
                            bool is_hamiltonian, vertex_t s, vertex_t t,
                            const setset* search_space);

setset SearchDirectedForests(const std::vector<edge_t>& digraph,
                             const std::vector<vertex_t>& roots,
                             bool is_spanning, const setset* search_space);

setset SearchRootedTrees(const std::vector<edge_t>& digraph, vertex_t root,
                         bool is_spanning, const setset* search_space);

setset SearchDirectedGraphs(
    const std::vector<edge_t>& digraph,
    const std::map<vertex_t, tdzdd::Range>* in_degree_constraints,
    const std::map<vertex_t, tdzdd::Range>* out_degree_constraints,
    const setset* search_space);

bool ShowMessages(bool flag = true);

}  // namespace digraphillion

#endif  // DIGRAPHILLION_GRAPHSET_H_
