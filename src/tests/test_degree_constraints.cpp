#include <iostream>

#include "graphset.h"
#include "spec/FrontierDegreeSpecified.hpp"
#include "subsetting/DdStructure.hpp"
#include "subsetting/util/IntRange.hpp"

int main() {
  tdzdd::Digraph graph;
  std::string file_name = "grid2x3.txt";
  graph.readEdges(file_name);

  FrontierManager fm(graph);
  // fm.print();

  std::cerr << "# of vertices = " << graph.vertexSize() << std::endl;
  std::cerr << "# of edges = " << graph.edgeSize() << std::endl;

  tdzdd::DdStructure<2> dd;

  FrontierDegreeSpecifiedSpec spec(graph);
  for (int i = 1; i <= 6; i++) {
    tdzdd::Range c(1, 2);
    std::cerr << "i:" << i << " vertex:" << graph.getVertex(std::to_string(i))
              << std::endl;
    spec.setIndegConstraint(graph.getVertex(std::to_string(i)), c);
    spec.setOutdegConstraint(graph.getVertex(std::to_string(i)), c);
  }
  dd = DdStructure<2>(spec);
  dd.zddReduce();
  std::cerr << "# of ZDD nodes = " << dd.size() << std::endl;
  std::cerr << "# of solutions = " << dd.zddCardinality() << std::endl;
}
