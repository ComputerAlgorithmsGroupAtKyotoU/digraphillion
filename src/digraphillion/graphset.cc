#include "digraphillion/graphset.h"

#include <assert.h>

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

bool ShowMessages(bool flag) { return MessageHandler::showMessages(flag); }

}  // namespace digraphillion
