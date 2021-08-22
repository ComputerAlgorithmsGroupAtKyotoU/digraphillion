/**
Copyright (c) 2021 ComputerAlgorithmsGroupAtKyotoU

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
