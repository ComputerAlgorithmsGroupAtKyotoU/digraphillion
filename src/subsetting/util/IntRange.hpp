#pragma once

#include <cassert>

#include "IntSubset.hpp"

namespace tdzdd {
class Range : public tdzdd::IntSubset {
 public:
  Range(int max = 1) : min_(0), max_(max - 1), step_(1) {
    assert(this->min_ <= this->max_);
  }
  Range(int min, int max, int step = 1) : min_(min), max_(max - 1), step_(step) {
    assert(this->min_ <= this->max_);
    assert(this->step_ > 0);
  }

  bool contains(int x) const {
    if (x < this->min_ || this->max_ < x) return false;
    return (x - this->min_) % this->step_ == 0;
  }
  int lowerBound() const { return this->min_; }
  int upperBound() const { return this->max_; }

 private:
  int min_;
  int max_;
  int step_;
};
}  // namespace tdzdd
