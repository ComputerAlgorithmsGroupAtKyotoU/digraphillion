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

#include "digraphillion/graphset.h"

#include <assert.h>

#include "spec/FrontierDegreeSpecified.hpp"
#include "spec/FrontierDirectedHamiltonianCycle.hpp"
#include "spec/FrontierDirectedSTPath.hpp"
#include "spec/FrontierDirectedSingleCycle.hpp"
#include "spec/FrontierRootedForest.hpp"
#include "spec/FrontierRootedTree.hpp"
#include "subsetting/DdStructure.hpp"
#include "subsetting/eval/ToZBDD.hpp"
#include "subsetting/spec/SapporoZdd.hpp"
#include "subsetting/util/MessageHandler.hpp"

namespace digraphillion {

using std::map;
using std::pair;
using std::set;
using std::vector;
using namespace tdzdd;

setset SearchDirectedCycles(const std::vector<edge_t>& digraph,
                            const setset* search_space) {
  assert(static_cast<size_t>(setset::num_elems()) == digraph.size());

  Digraph g;
  for (vector<edge_t>::const_iterator e = digraph.begin(); e != digraph.end();
       ++e)
    g.addEdge(e->first, e->second);
  g.update();
  assert(static_cast<size_t>(g.edgeSize()) == digraph.size());

  DdStructure<2> dd;
  if (search_space != NULL) {
    SapporoZdd f(search_space->zdd_, setset::max_elem() - setset::num_elems());
    dd = DdStructure<2>(f);
  } else {
    dd = DdStructure<2>(g.edgeSize());
  }

  FrontierDirectedSingleCycleSpec spec(g);
  dd.zddSubset(spec);
  dd.zddReduce();

  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}

setset SearchDirectedHamiltonianCycles(const std::vector<edge_t>& digraph,
                                       const setset* search_space) {
  assert(static_cast<size_t>(setset::num_elems()) == digraph.size());

  Digraph g;
  for (vector<edge_t>::const_iterator e = digraph.begin(); e != digraph.end();
       ++e)
    g.addEdge(e->first, e->second);
  g.update();
  assert(static_cast<size_t>(g.edgeSize()) == digraph.size());

  DdStructure<2> dd;
  if (search_space != NULL) {
    SapporoZdd f(search_space->zdd_, setset::max_elem() - setset::num_elems());
    dd = DdStructure<2>(f);
  } else {
    dd = DdStructure<2>(g.edgeSize());
  }

  FrontierDirectedSingleHamiltonianCycleSpec spec(g);
  dd.zddSubset(spec);
  dd.zddReduce();

  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}

setset SearchDirectedSTPath(const std::vector<edge_t>& digraph,
                            bool is_hamiltonian, vertex_t s, vertex_t t,
                            const setset* search_space) {
  assert(static_cast<size_t>(setset::num_elems()) == digraph.size());

  Digraph g;
  for (vector<edge_t>::const_iterator e = digraph.begin(); e != digraph.end();
       ++e)
    g.addEdge(e->first, e->second);
  g.update();
  assert(static_cast<size_t>(g.edgeSize()) == digraph.size());

  DdStructure<2> dd;
  if (search_space != NULL) {
    SapporoZdd f(search_space->zdd_, setset::max_elem() - setset::num_elems());
    dd = DdStructure<2>(f);
  } else {
    dd = DdStructure<2>(g.edgeSize());
  }

  FrontierDirectedSTPathSpec spec(g, is_hamiltonian, g.getVertex(s),
                                  g.getVertex(t));
  dd.zddSubset(spec);
  dd.zddReduce();

  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}

setset SearchDirectedForests(const std::vector<edge_t>& digraph,
                             const std::vector<vertex_t>& roots,
                             bool is_spanning, const setset* search_space) {
  assert(static_cast<size_t>(setset::num_elems()) == digraph.size());

  Digraph g;
  for (vector<edge_t>::const_iterator e = digraph.begin(); e != digraph.end();
       ++e)
    g.addEdge(e->first, e->second);
  g.update();
  assert(static_cast<size_t>(g.edgeSize()) == digraph.size());

  std::set<tdzdd::Digraph::VertexNumber> roots_set;
  for (const auto& root : roots) {
    roots_set.insert(g.getVertex(root));
  }

  DdStructure<2> dd;
  if (search_space != NULL) {
    SapporoZdd f(search_space->zdd_, setset::max_elem() - setset::num_elems());
    dd = DdStructure<2>(f);
  } else {
    dd = DdStructure<2>(g.edgeSize());
  }

  FrontierRootedForestSpec spec(g, roots_set, is_spanning);
  dd.zddSubset(spec);
  dd.zddReduce();

  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}

setset SearchRootedTrees(const std::vector<edge_t>& digraph, vertex_t root,
                         bool is_spanning, const setset* search_space) {
  assert(static_cast<size_t>(setset::num_elems()) == digraph.size());

  Digraph g;
  for (vector<edge_t>::const_iterator e = digraph.begin(); e != digraph.end();
       ++e)
    g.addEdge(e->first, e->second);
  g.update();
  assert(static_cast<size_t>(g.edgeSize()) == digraph.size());

  DdStructure<2> dd;
  if (search_space != NULL) {
    SapporoZdd f(search_space->zdd_, setset::max_elem() - setset::num_elems());
    dd = DdStructure<2>(f);
  } else {
    dd = DdStructure<2>(g.edgeSize());
  }

  FrontierRootedTreeSpec spec(g, g.getVertex(root), is_spanning);
  dd.zddSubset(spec);
  dd.zddReduce();

  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}

setset SearchDirectedGraphs(
    const std::vector<edge_t>& digraph,
    const std::map<vertex_t, Range>* in_degree_constraints,
    const std::map<vertex_t, Range>* out_degree_constraints,
    const setset* search_space) {
  assert(static_cast<size_t>(setset::num_elems()) == digraph.size());

  Digraph g;
  for (vector<edge_t>::const_iterator e = digraph.begin(); e != digraph.end();
       ++e)
    g.addEdge(e->first, e->second);
  g.update();
  assert(static_cast<size_t>(g.edgeSize()) == digraph.size());

  DdStructure<2> dd;
  if (search_space != NULL) {
    SapporoZdd f(search_space->zdd_, setset::max_elem() - setset::num_elems());
    dd = DdStructure<2>(f);
  } else {
    dd = DdStructure<2>(g.edgeSize());
  }

  FrontierDegreeSpecifiedSpec in_dc(g);
  if (in_degree_constraints != NULL) {
    for (auto i = in_degree_constraints->begin();
         i != in_degree_constraints->end(); ++i) {
      in_dc.setIndegConstraint(g.getVertex(i->first), i->second);
    }
    dd.zddSubset(in_dc);
    dd.zddReduce();
  }

  FrontierDegreeSpecifiedSpec out_dc(g);
  if (out_degree_constraints != NULL) {
    for (auto i = out_degree_constraints->begin();
         i != out_degree_constraints->end(); i++) {
      out_dc.setOutdegConstraint(g.getVertex(i->first), i->second);
    }
    dd.zddSubset(out_dc);
    dd.zddReduce();
  }

  zdd_t f = dd.evaluate(ToZBDD(setset::max_elem() - setset::num_elems()));
  return setset(f);
}

bool ShowMessages(bool flag) { return MessageHandler::showMessages(flag); }

}  // namespace digraphillion
