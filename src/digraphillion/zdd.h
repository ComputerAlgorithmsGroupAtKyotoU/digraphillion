#ifndef DIGRAPHILLION_ZDD_H_
#define DIGRAPHILLION_ZDD_H_

#include <assert.h>

#include <map>
#include <set>

#include "digraphillion/type.h"

namespace digraphillion {

void init();
elem_t elem_limit();
elem_t max_elem();
void new_elems(elem_t max_elem);
elem_t num_elems();
void num_elems(elem_t num_elems);

zdd_t single(elem_t e);
inline word_t id(zdd_t f) { return f.GetID(); }
inline zdd_t null() { return zdd_t(-1); }
inline zdd_t bot() { return zdd_t(0); }
inline zdd_t top() { return zdd_t(1); }
inline bool is_term(zdd_t f) { return f.Top() == 0; }
inline zdd_t lo(zdd_t f) {
  assert(!is_term(f));
  return f.OffSet(f.Top());
}
inline zdd_t hi(zdd_t f) {
  assert(!is_term(f));
  return f.OnSet0(f.Top());
}
inline elem_t elem(zdd_t f) {
  assert(!is_term(f));
  return f.Top();
}

zdd_t operator|(const zdd_t& f, const zdd_t& g);

zdd_t complement(zdd_t f);
zdd_t minimal(zdd_t f);
zdd_t maximal(zdd_t f);
zdd_t hitting(zdd_t f);
zdd_t join(zdd_t f, zdd_t g);
zdd_t meet(zdd_t f, zdd_t g);
zdd_t non_subsets(zdd_t f, zdd_t g);
zdd_t non_supersets(zdd_t f, zdd_t g);
bool choose(zdd_t f, std::vector<elem_t>* stack);
zdd_t choose_random(zdd_t f, std::vector<elem_t>* stack);
zdd_t choose_best(zdd_t f, const std::vector<double>& weights,
                  std::set<elem_t>* s);
void dump(zdd_t f, std::ostream& out);
void dump(zdd_t f, FILE* fp = stdout);
zdd_t load(std::istream& in);
zdd_t load(FILE* fp = stdin);
void _enum(zdd_t f, std::ostream& out,
           const std::pair<const char*, const char*>& outer_braces,
           const std::pair<const char*, const char*>& inner_braces);
void _enum(zdd_t f, FILE* fp,
           const std::pair<const char*, const char*>& outer_braces,
           const std::pair<const char*, const char*>& inner_braces);
void _enum(zdd_t f, std::ostream& out, std::vector<elem_t>* stack, bool* first,
           const std::pair<const char*, const char*>& inner_braces);
void _enum(zdd_t f, FILE* fp, std::vector<elem_t>* stack, bool* first,
           const std::pair<const char*, const char*>& inner_braces);
void algo_b(zdd_t f, const std::vector<double>& w, std::vector<bool>* x);
double algo_c(zdd_t f);
double probability(elem_t e, zdd_t f, const std::vector<double>& probabilities,
                   std::map<word_t, double>& cache);
zdd_t zuniq(elem_t v, zdd_t l, zdd_t h);
double rand_xor128();
void sort_zdd(zdd_t f, std::vector<std::vector<zdd_t> >* stacks,
              std::set<word_t>* visited, elem_t* max_elem = NULL);
inline std::pair<word_t, word_t> make_key(zdd_t f, zdd_t g) {
  return std::make_pair(id(f), id(g));
}

}  // namespace digraphillion

#endif  // DIGRAPHILLION_ZDD_H_
