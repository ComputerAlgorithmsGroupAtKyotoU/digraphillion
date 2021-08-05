#ifndef DIGRAPHILLION_FRONTIER_DATA_HPP_
#define DIGRAPHILLION_FRONTIER_DATA_HPP_

// data associated with each vertex on the frontier
class DirectedFrontierData {
 public:
  short indeg;
  short outdeg;
  short comp;
};

#endif
