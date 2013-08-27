#ifndef __CXXMPH_POWER_INDEX_H__
#define __CXXMPH_POWER_INDEX_H__

// Split from power_map.h. Indexing class with low probability success. Accepts
// a cost model to define which positions are better left empty.

#include <cassert>
#include <cmath>
#include <set>
#include <vector>

#include "seeded_hash.h"
#include "string_util.h"

namespace cxxmph {

using std::set;
using std::vector;

inline uint8_t power_hash(const h128& h, uint16_t ph) {
  uint8_t a = h[3] >> (ph & 31);
  uint8_t pos = (a * ph) >> 8;
  return pos;
}

// Perfect hash function for fixed size data structures and pre-hashed keys.
class power_index_h128 {
 public:
  uint32_t index(const h128& h) { return power_hash(h, perfect_hash_); }
  bool Reset(
      h128* begin, h128* end, uint16_t* cost_begin, uint16_t* cost_end);
  void clear();

 private:
  uint8_t perfect_hash_;
};

// Add size() and generalize
class power_index {
 public:
  power_index(uint8_t log2capacity) : capacity_(1L << log2capacity), nkeys_(0) {}
  template <class SeededHashFcn, class ForwardIterator>
  bool Reset(ForwardIterator begin, ForwardIterator end, uint8_t nkeys);
  template <class SeededHashFcn, class Key>  // must agree with Reset
  // Get a unique identifier for k, in the range [0;size()). If x wasn't part
  // of the input in the last Reset call, returns a random value.
  uint8_t index(const Key& x) const;
  uint16_t capacity() const { return capacity_; }
  uint32_t size() const { return nkeys_; }
  void clear();

 private:
  const uint16_t capacity_;
  uint8_t nkeys_;
  power_index_h128 index_;
};

template <class Key, class HashFcn = typename seeded_hash<std::hash<Key>>::hash_function>
class simple_power_index : public power_index {
 public:
  simple_power_index(uint8_t log2capacity) : power_index(log2capacity) {}
  template <class ForwardIterator>
  bool Reset(ForwardIterator begin, ForwardIterator end, uint8_t nkeys) {
    return power_index::Reset<HashFcn, ForwardIterator>(begin, end, nkeys);
  }
  uint32_t index(const Key& key) const { return power_index::index<HashFcn>(key); }
};

template <class SeededHashFcn, class ForwardIterator>
bool power_index::Reset(ForwardIterator begin, ForwardIterator end, uint8_t nkeys) {
  SeededHashFcn hasher;
  uint32_t seed = 3;
  set<h128> hashed_keys;
  for (auto it = begin; it != end; ++it) {
    auto h = hasher.hash128(*it, seed);
    if (!hashed_keys.insert(h).second) return false;
  }
  if (nkeys == 0) { nkeys_ = 0; return true; }
  assert(hashed_keys.size() == nkeys);

  vector<h128> keys(hashed_keys.begin(), hashed_keys.end());
  assert(keys.size());
  auto key_begin = &(keys[0]);
  auto key_end = key_begin + keys.size();

  vector<uint16_t> cost(capacity_);
  for (uint16_t i = 0; i < cost.size(); ++i) cost[i] = i;
  assert(cost.size());
  auto cost_begin = &(cost[0]);
  auto cost_end = cost_begin + cost.size();
  CXXMPH_DEBUGLN("Generating a ph for %v keys at [0;%d]")(nkeys, cost.size());

  bool ok = index_.Reset(key_begin, key_end,
                         cost_begin, cost_end);
  if (!ok) return false;

  nkeys_ = nkeys;
  return ok;
}

}  // namespace cxxmph

#endif
