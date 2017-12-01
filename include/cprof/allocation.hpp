#ifndef ALLOCATIONRECORD_HPP
#define ALLOCATIONRECORD_HPP

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>

#include <boost/icl/interval_set.hpp>

#include "address_space.hpp"
#include "cprof/model/location.hpp"
#include "cprof/model/memory.hpp"
#include "cprof/model/thread.hpp"

using namespace boost::icl;

class Allocation {

public:
  typedef uint64_t id_type;

private:
  uintptr_t pos_;
  size_t size_;
  AddressSpace address_space_;
  cprof::model::Memory memory_;
  cprof::model::tid_t thread_id_;
  cprof::model::Location location_;

  id_type id_;
  bool freed_;

  static id_type unique_id() {
    static id_type count = 0;
    return count++;
  }

public:
  Allocation(const uintptr_t pos, const size_t size)
      : pos_(pos), size_(size), address_space_(AddressSpace::Host()),
        location_(cprof::model::Location::Host()) {}
  Allocation(uintptr_t pos, size_t size, const AddressSpace &as,
             const cprof::model::Memory &mem,
             const cprof::model::Location &location)
      : pos_(pos), size_(size), address_space_(as), memory_(mem),
        location_(location), id_(unique_id()), freed_(false) {
    assert(address_space_.is_valid());
  }
  Allocation() : Allocation(0, 0) {}

  std::string json() const;

  id_type id() const { return id_; }
  AddressSpace address_space() const { return address_space_; }
  cprof::model::Memory memory() const { return memory_; }

  void mark_free() { freed_ = true; }
  bool freed() const { return freed_; }

  uintptr_t pos() const noexcept { return pos_; }
  size_t size() const noexcept { return size_; }

  explicit operator bool() const noexcept { return pos_; }
};

namespace boost {
namespace icl {

template <> struct interval_traits<Allocation> {

  typedef Allocation interval_type;
  typedef uintptr_t domain_type;
  typedef std::less<uintptr_t> domain_compare;
  static interval_type construct(const domain_type &lo, const domain_type &up) {
    return interval_type(lo, up - lo);
  }
  // 3.2 Selection of values
  static domain_type lower(const interval_type &inter_val) {
    return inter_val.pos();
  };
  static domain_type upper(const interval_type &inter_val) {
    return inter_val.pos() + inter_val.size();
  };
};

template <>
struct interval_bound_type<Allocation> // 4.  Finally we define the interval
                                       // borders.
{ //    Choose between static_open         (lo..up)
  typedef interval_bound_type
      type; //                   static_left_open    (lo..up]
  BOOST_STATIC_CONSTANT(bound_type, value = interval_bounds::static_right_open);
}; //               and static_closed       [lo..up]

} // namespace icl
} // namespace boost

#endif
