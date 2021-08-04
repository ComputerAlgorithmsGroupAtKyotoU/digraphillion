#ifndef DIGRAPHILLION_SETSET_H_
#define DIGRAPHILLION_SETSET_H_

#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "digraphillion/type.h"

namespace digraphillion {

class Range;

class setset {
 public:
  class iterator
      : public std::iterator<std::forward_iterator_tag, std::set<elem_t> > {
   public:
    iterator();
    iterator(const iterator& i);
    explicit iterator(const setset& ss);
    explicit iterator(const setset& ss, const std::set<elem_t>& s);

    virtual ~iterator() {}

    bool operator==(const iterator& i) const { return this->zdd_ == i.zdd_; }
    bool operator!=(const iterator& i) const { return this->zdd_ != i.zdd_; }
    std::set<elem_t>& operator*() { return this->s_; }
    iterator& operator++();

   protected:
    virtual void next();

    zdd_t zdd_;
    std::set<elem_t> s_;
  };

  typedef iterator const_iterator;

  class random_iterator : public iterator {
   public:
    random_iterator();
    random_iterator(const random_iterator& i);
    explicit random_iterator(const setset& ss);

    virtual ~random_iterator() {}

   protected:
    virtual void next();

    double size_;
  };

  class weighted_iterator : public iterator {
   public:
    weighted_iterator();
    weighted_iterator(const weighted_iterator& i);
    explicit weighted_iterator(const setset& ss, std::vector<double> weights);

    virtual ~weighted_iterator() {}

   protected:
    virtual void next();

    std::vector<double> weights_;
  };

  setset();
  setset(const setset& ss) : zdd_(ss.zdd_) {}
  explicit setset(const std::vector<std::set<elem_t> >& v);
  explicit setset(const std::map<std::string, std::vector<elem_t> >& m);
  explicit setset(std::istream& in);

  virtual ~setset() {}

  void operator=(const setset& ss) { this->zdd_ = ss.zdd_; }
  bool operator==(const setset& ss) { return this->zdd_ == ss.zdd_; }
  bool operator!=(const setset& ss) { return this->zdd_ != ss.zdd_; }
  setset operator~() const;
  setset operator&(const setset& ss) const;
  setset operator|(const setset& ss) const;
  setset operator-(const setset& ss) const;
  setset operator^(const setset& ss) const;
  setset operator/(const setset& ss) const;
  setset operator%(const setset& ss) const;
  void operator|=(const setset& ss);
  void operator&=(const setset& ss);
  void operator-=(const setset& ss);
  void operator^=(const setset& ss);
  void operator/=(const setset& ss);
  void operator%=(const setset& ss);
  bool operator<=(const setset& ss) const;
  bool operator<(const setset& ss) const;
  bool operator>=(const setset& ss) const;
  bool operator>(const setset& ss) const;

  word_t id() const;

  bool is_disjoint(const setset& ss) const;
  bool is_subset(const setset& ss) const;
  bool is_superset(const setset& ss) const;

  bool empty() const;
  std::string size() const;
  iterator begin() const;
  random_iterator begin_randomly() const;
  weighted_iterator begin_from_min(const std::vector<double>& weights) const;
  weighted_iterator begin_from_max(const std::vector<double>& weights) const;
  iterator find(const std::set<elem_t>& s) const;
  size_t count(const std::set<elem_t>& s) const;
  std::pair<iterator, bool> insert(const std::set<elem_t>& s);
  iterator insert(const_iterator hint, const std::set<elem_t>& s);
  void insert(elem_t e);
  iterator erase(const_iterator position);
  size_t erase(const std::set<elem_t>& s);
  void erase(elem_t e);
  void clear();
  void swap(setset& ss);
  void flip(elem_t e);
  void flip();

  setset minimal() const;
  setset maximal() const;
  setset hitting() const;
  setset smaller(size_t set_size) const;
  setset larger(size_t set_size) const;
  setset set_size(size_t set_size) const;

  setset join(const setset& ss) const;
  setset meet(const setset& ss) const;
  setset subsets(const setset& ss) const;
  setset supersets(const setset& ss) const;
  setset supersets(elem_t e) const;
  setset non_subsets(const setset& ss) const;
  setset non_supersets(const setset& ss) const;
  setset non_supersets(elem_t e) const;

  double probability(const std::vector<double>& probabilities) const;

  void dump(std::ostream& out) const;
  void dump(FILE* fp = stdout) const;
  static setset load(std::istream& in);
  static setset load(FILE* fp = stdin);
  void _enum(std::ostream& out,
             const std::pair<const char*, const char*> outer_braces =
                 std::make_pair("{", "}"),
             const std::pair<const char*, const char*> inner_braces =
                 std::make_pair("{", "}")) const;
  void _enum(FILE* fp = stdout,
             const std::pair<const char*, const char*> outer_braces =
                 std::make_pair("{", "}"),
             const std::pair<const char*, const char*> inner_braces =
                 std::make_pair("{", "}")) const;

  static iterator end() { return iterator(); }
  static elem_t elem_limit();
  static elem_t max_elem();
  static elem_t num_elems();
  static void num_elems(elem_t num_elems);

  friend std::ostream& operator<<(std::ostream& out, const setset& ss);
  friend std::istream& operator>>(std::istream& in, setset& ss);

 private:
  explicit setset(const std::set<elem_t>& s);
  explicit setset(const zdd_t& z) : zdd_(z) {}

  zdd_t zdd_;

  friend class TestSetset;
  friend setset SearchDirectedCycles(const std::vector<edge_t>& digraph,
                                     const setset* search_space);
};

}  // namespace digraphillion

#endif  // DIGRAPHILLION_SETSET_H_
