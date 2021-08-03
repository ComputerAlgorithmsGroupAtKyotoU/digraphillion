#include "digraphillion/setset.h"

#include <algorithm>

#include "digraphillion/zdd.h"
#include "subsetting/dd/PathCounter.hpp"
#include "subsetting/spec/SapporoZdd.hpp"

namespace digraphillion {

using std::istream;
using std::make_pair;
using std::map;
using std::ostream;
using std::pair;
using std::set;
using std::sort;
using std::string;
using std::vector;

// setset::iterator

setset::iterator::iterator() : zdd_(null()), s_(set<elem_t>()) {}

setset::iterator::iterator(const setset::iterator& i)
    : zdd_(i.zdd_), s_(i.s_) {}

setset::iterator::iterator(const setset& ss)
    : zdd_(ss.zdd_), s_(set<elem_t>()) {
  this->next();
}

setset::iterator::iterator(const setset& ss, const set<elem_t>& s)
    : zdd_(ss.zdd_), s_(s) {}

setset::iterator& setset::iterator::operator++() {
  this->next();
  return *this;
}

void setset::iterator::next() {
  if (this->zdd_ == null() || this->zdd_ == bot()) {
    this->zdd_ = null();
    this->s_ = set<elem_t>();
  } else {
    vector<elem_t> stack(this->s_.begin(), this->s_.end());
    sort(stack.begin(), stack.end());
    bool res = choose(this->zdd_, &stack);
    if (res) {
      this->s_ = set<elem_t>(stack.begin(), stack.end());
      if (stack.size() == 0)  // reach to the empty set (the last)
        this->zdd_ = bot();
    } else {
      this->zdd_ = null();
    }
  }
}

setset::random_iterator::random_iterator() : iterator(), size_(0) {}

setset::random_iterator::random_iterator(const setset::random_iterator& i)
    : iterator(i), size_(i.size_) {}

setset::random_iterator::random_iterator(const setset& ss)
    : iterator(ss, set<elem_t>()) {
  this->size_ = algo_c(ss.zdd_);
  this->next();
}

void setset::random_iterator::next() {
  if (this->zdd_ == null() || this->zdd_ == bot()) {
    this->zdd_ = null();
    this->s_ = set<elem_t>();
  } else {
    vector<elem_t> stack;
    zdd_t z = choose_random(this->zdd_, &stack);
    // Since same sets are rarely selected from very large setset that has more
    // than 1e17 sets, we don't need to remove the selected sets
    if (this->size_ < 1e17) this->zdd_ -= z;
    this->s_ = set<elem_t>(stack.begin(), stack.end());
  }
}

setset::weighted_iterator::weighted_iterator() : iterator() {}

setset::weighted_iterator::weighted_iterator(const setset::weighted_iterator& i)
    : iterator(i), weights_(i.weights_) {}

setset::weighted_iterator::weighted_iterator(const setset& ss,
                                             vector<double> weights)
    : iterator(ss, set<elem_t>()), weights_(weights) {
  this->next();
}

void setset::weighted_iterator::next() {
  if (this->zdd_ == null() || this->zdd_ == bot()) {
    this->zdd_ = null();
    this->s_ = set<elem_t>();
  } else {
    set<elem_t> s;
    this->zdd_ -= choose_best(this->zdd_, this->weights_, &s);
    this->s_ = s;
  }
}

// setset

setset::setset() : zdd_(bot()) {}

setset::setset(const vector<set<elem_t> >& v) : zdd_(bot()) {
  for (vector<set<elem_t> >::const_iterator s = v.begin(); s != v.end(); ++s)
    this->zdd_ += setset(*s).zdd_;
}

setset::setset(const map<string, vector<elem_t> >& m) {
  for (map<string, vector<elem_t> >::const_iterator i = m.begin(); i != m.end();
       ++i)
    assert(i->first == "include" || i->first == "exclude");
  map<string, vector<elem_t> >::const_iterator in_i = m.find("include");
  map<string, vector<elem_t> >::const_iterator ex_i = m.find("exclude");
  const vector<elem_t>& in_v =
      in_i != m.end() ? in_i->second : vector<elem_t>();
  const vector<elem_t>& ex_v =
      ex_i != m.end() ? ex_i->second : vector<elem_t>();
  for (vector<elem_t>::const_iterator e = in_v.begin(); e != in_v.end(); ++e)
    single(*e);
  for (vector<elem_t>::const_iterator e = ex_v.begin(); e != ex_v.end(); ++e)
    single(*e);
  vector<zdd_t> n(num_elems() + 2);
  n[0] = bot(), n[1] = top();
  for (elem_t v = num_elems(); v > 0; --v) {
    bool in_found = std::find(in_v.begin(), in_v.end(), v) != in_v.end();
    bool ex_found = std::find(ex_v.begin(), ex_v.end(), v) != ex_v.end();
    assert(!(in_found && ex_found));
    elem_t i = num_elems() - v + 2;
    n[i] = in_found   ? n[0] + single(v) * n[i - 1]
           : ex_found ? n[i - 1] + single(v) * n[0]
                      : n[i - 1] + single(v) * n[i - 1];
  }
  this->zdd_ = n[num_elems() + 1];
}

setset::setset(istream& in) : zdd_(digraphillion::load(in)) {}

setset setset::operator~() const { return setset(complement(this->zdd_)); }

setset setset::operator|(const setset& ss) const {
  return setset(this->zdd_ + ss.zdd_);
}

setset setset::operator&(const setset& ss) const {
  return setset(this->zdd_ & ss.zdd_);
}

setset setset::operator-(const setset& ss) const {
  return setset(this->zdd_ - ss.zdd_);
}

setset setset::operator^(const setset& ss) const {
  return setset((this->zdd_ - ss.zdd_) + (ss.zdd_ - this->zdd_));
}

setset setset::operator/(const setset& ss) const {
  assert(ss.zdd_ != bot() || is_term(this->zdd_));
  return setset(this->zdd_ / ss.zdd_);
}

setset setset::operator%(const setset& ss) const {
  assert(ss.zdd_ != bot() || is_term(this->zdd_));
  return setset(this->zdd_ % ss.zdd_);
}

void setset::operator&=(const setset& ss) { this->zdd_ &= ss.zdd_; }

void setset::operator|=(const setset& ss) { this->zdd_ += ss.zdd_; }

void setset::operator-=(const setset& ss) { this->zdd_ -= ss.zdd_; }

void setset::operator^=(const setset& ss) {
  this->zdd_ = (this->zdd_ - ss.zdd_) + (ss.zdd_ - this->zdd_);
}

void setset::operator/=(const setset& ss) { this->zdd_ /= ss.zdd_; }

void setset::operator%=(const setset& ss) { this->zdd_ %= ss.zdd_; }

bool setset::operator<=(const setset& ss) const {
  return (this->zdd_ - ss.zdd_) == bot();
}

bool setset::operator<(const setset& ss) const {
  return (this->zdd_ - ss.zdd_) == bot() && this->zdd_ != ss.zdd_;
}

bool setset::operator>=(const setset& ss) const {
  return (ss.zdd_ - this->zdd_) == bot();
}

bool setset::operator>(const setset& ss) const {
  return (ss.zdd_ - this->zdd_) == bot() && this->zdd_ != ss.zdd_;
}

word_t setset::id() const { return digraphillion::id(this->zdd_); }

bool setset::is_disjoint(const setset& ss) const {
  return (this->zdd_ & ss.zdd_) == bot();
}

bool setset::is_subset(const setset& ss) const { return *this <= ss; }

bool setset::is_superset(const setset& ss) const { return *this >= ss; }

bool setset::empty() const { return this->zdd_ == bot(); }

string setset::size() const {
  tdzdd::SapporoZdd f(this->zdd_,
                      digraphillion::max_elem() - digraphillion::num_elems());
  return countPaths(f, true);
}

setset::iterator setset::begin() const { return setset::iterator(*this); }

setset::random_iterator setset::begin_randomly() const {
  return setset::random_iterator(*this);
}

setset::weighted_iterator setset::begin_from_min(
    const vector<double>& weights) const {
  vector<double> inverted_weights;
  for (vector<double>::const_iterator i = weights.begin(); i != weights.end();
       ++i)
    inverted_weights.push_back(-1 * (*i));
  return setset::weighted_iterator(*this, inverted_weights);
}

setset::weighted_iterator setset::begin_from_max(
    const vector<double>& weights) const {
  return setset::weighted_iterator(*this, weights);
}

setset::iterator setset::find(const set<elem_t>& s) const {
  if (this->zdd_ - setset(s).zdd_ != this->zdd_)
    return setset::iterator(*this, s);
  else
    return setset::iterator();
}

size_t setset::count(const set<elem_t>& s) const {
  return this->zdd_ / setset(s).zdd_ != bot() ? 1 : 0;
}

pair<setset::iterator, bool> setset::insert(const set<elem_t>& s) {
  if (this->find(s) != end()) {
    return make_pair(setset::iterator(*this, s), false);
  } else {
    *this |= setset(s);
    return make_pair(setset::iterator(*this, s), true);
  }
}

setset::iterator setset::insert(const_iterator /*hint*/, const set<elem_t>& s) {
  pair<iterator, bool> p = this->insert(s);
  return p.first;
}

void setset::insert(elem_t e) {
  set<elem_t> s;
  s.insert(e);
  this->zdd_ = digraphillion::join(this->zdd_, setset(s).zdd_);
}

setset::iterator setset::erase(const_iterator position) {
  this->erase(*position);
  return setset::iterator();
}

size_t setset::erase(const set<elem_t>& s) {
  if (this->find(s) != end()) {
    *this -= setset(s);
    return 1;
  } else {
    return 0;
  }
}

void setset::erase(elem_t e) {
  set<elem_t> s;
  for (elem_t e2 = 1; e2 <= num_elems(); ++e2)
    if (e != e2) s.insert(e2);
  this->zdd_ = digraphillion::meet(this->zdd_, setset(s).zdd_);
}

void setset::clear() { this->zdd_ = bot(); }

void setset::swap(setset& ss) {
  zdd_t z = this->zdd_;
  this->zdd_ = ss.zdd_;
  ss.zdd_ = z;
}

void setset::flip(elem_t e) { this->zdd_ = this->zdd_.Change(e); }

void setset::flip() {
  for (elem_t e = 1; e <= digraphillion::num_elems(); ++e)
    this->zdd_ = this->zdd_.Change(e);
}

setset setset::minimal() const {
  return setset(digraphillion::minimal(this->zdd_));
}

setset setset::maximal() const {
  return setset(digraphillion::maximal(this->zdd_));
}

setset setset::hitting() const {  // a.k.a cross elements
  return setset(digraphillion::hitting(this->zdd_));
}

setset setset::smaller(size_t set_size) const {
  return setset(this->zdd_.PermitSym(set_size - 1));
}

setset setset::larger(size_t set_size) const {
  return setset(this->zdd_ - this->zdd_.PermitSym(set_size));
}

setset setset::set_size(size_t set_size) const {
  zdd_t z = this->zdd_.PermitSym(set_size) - this->zdd_.PermitSym(set_size - 1);
  return setset(z);
}

setset setset::join(const setset& ss) const {
  return setset(digraphillion::join(this->zdd_, ss.zdd_));
}

setset setset::meet(const setset& ss) const {
  return setset(digraphillion::meet(this->zdd_, ss.zdd_));
}

setset setset::subsets(const setset& ss) const {
  return setset(this->zdd_.Permit(ss.zdd_));
}

setset setset::supersets(const setset& ss) const {
  return setset(this->zdd_.Restrict(ss.zdd_));
}

setset setset::supersets(elem_t e) const {
  set<elem_t> s;
  s.insert(e);
  zdd_t z1 = setset(s).zdd_;
  zdd_t z2 = this->zdd_ / z1;
  return setset(z2 * z1);
}

setset setset::non_subsets(const setset& ss) const {
  return setset(digraphillion::non_subsets(this->zdd_, ss.zdd_));
}

setset setset::non_supersets(const setset& ss) const {
  return setset(digraphillion::non_supersets(this->zdd_, ss.zdd_));
}

setset setset::non_supersets(elem_t e) const {
  set<elem_t> s;
  s.insert(e);
  return setset(this->zdd_ % setset(s).zdd_);
}

double setset::probability(const vector<double>& probabilities) const {
  assert(probabilities.size() == num_elems() + 1);
  if (this->zdd_ == bot()) {  // this->empty()
    return 0;
  } else if (this->zdd_ == top()) {
    double p = 1;
    for (int e = 1; e <= num_elems(); ++e) p *= 1 - probabilities[e];
    return p;
  } else {
    map<word_t, double> cache;
    cache[digraphillion::id(bot())] = 0;
    cache[digraphillion::id(top())] = 1;
    return digraphillion::probability(1, this->zdd_, probabilities, cache);
  }
}

void setset::dump(ostream& out) const { digraphillion::dump(this->zdd_, out); }

void setset::dump(FILE* fp) const { digraphillion::dump(this->zdd_, fp); }

setset setset::load(istream& in) { return setset(digraphillion::load(in)); }

setset setset::load(FILE* fp) { return setset(digraphillion::load(fp)); }

void setset::_enum(ostream& out,
                   const pair<const char*, const char*> outer_braces,
                   const pair<const char*, const char*> inner_braces) const {
  digraphillion::_enum(this->zdd_, out, outer_braces, inner_braces);
}

void setset::_enum(FILE* fp, const pair<const char*, const char*> outer_braces,
                   const pair<const char*, const char*> inner_braces) const {
  digraphillion::_enum(this->zdd_, fp, outer_braces, inner_braces);
}

elem_t setset::elem_limit() { return digraphillion::elem_limit(); }

elem_t setset::max_elem() { return digraphillion::max_elem(); }

elem_t setset::num_elems() { return digraphillion::num_elems(); }

void setset::num_elems(elem_t num_elems) {
  digraphillion::num_elems(num_elems);
}

ostream& operator<<(ostream& out, const setset& ss) {
  digraphillion::dump(ss.zdd_, out);
  return out;
}

istream& operator>>(istream& in, setset& ss) {
  ss.zdd_ = digraphillion::load(in);
  return in;
}

setset::setset(const set<elem_t>& s) : zdd_(top()) {
  for (set<elem_t>::const_iterator e = s.begin(); e != s.end(); ++e)
    this->zdd_ *= single(*e);
}

}  // namespace digraphillion
