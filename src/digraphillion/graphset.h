#ifndef DIGRAPHILLION_GRAPHSET_H_
#define DIGRAPHILLION_GRAPHSET_H_

#include "subsetting/util/IntSubset.hpp"
#include "digraphillion/setset.h"

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

bool ShowMessages(bool flag = true);

}  // namespace digraphillion

#endif  // DIGRAPHILLION_GRAPHSET_H_
