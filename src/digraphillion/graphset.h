/*********************************************************************
Copyright 2013  JST ERATO Minato project and other contributors
http://www-erato.ist.hokudai.ac.jp/?language=en
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**********************************************************************/

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
