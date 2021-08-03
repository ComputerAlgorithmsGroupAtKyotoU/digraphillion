#ifndef DIGRAPHILLION_TYPE_H_
#define DIGRAPHILLION_TYPE_H_

#include <stdint.h>

#include <vector>

#include "SAPPOROBDD/ZBDD.h"

namespace digraphillion {

typedef ZBDD zdd_t;
typedef bddword word_t;
typedef int32_t elem_t;  // bddvar

typedef std::string vertex_t;
typedef std::pair<vertex_t, vertex_t> edge_t;
typedef std::pair<edge_t, double> weighted_edge_t;
typedef std::pair<std::vector<weighted_edge_t>, std::pair<double, double> >
    linear_constraint_t;

}  // namespace digraphillion

#endif  // DIGRAPHILLION_TYPE_H_
