#include "digraphillion/graphset.h"

#include <assert.h>

#include "spec/FrontierDirectedHamiltonianCycle.hpp"
#include "spec/FrontierDirectedSingleCycle.hpp"
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

Range::Range(int max) : min_(0), max_(max - 1), step_(1) {
  assert(this->min_ <= this->max_);
}

Range::Range(int min, int max, int step)
    : min_(min), max_(max - 1), step_(step) {
  assert(this->min_ <= this->max_);
  assert(this->step_ > 0);
}

bool Range::contains(int x) const {
  if (x < this->min_ || this->max_ < x) return false;
  return (x - this->min_) % this->step_ == 0;
}

int Range::lowerBound() const { return this->min_; }

int Range::upperBound() const { return this->max_; }

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

bool ShowMessages(bool flag) { return MessageHandler::showMessages(flag); }

}  // namespace digraphillion
