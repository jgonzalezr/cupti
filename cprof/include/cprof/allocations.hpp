#ifndef CPROF_ALLOCATIONS_HPP
#define CPROF_ALLOCATIONS_HPP

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>

#include <boost/icl/interval_map.hpp>

#include "address_space.hpp"
#include "allocation.hpp"
#include "cprof/model/location.hpp"
#include "util/extent.hpp"
#include "util/logging.hpp"

namespace cprof {
class Allocations {

public:
private:
  typedef uintptr_t pos_type;
  typedef Allocation value_type;
  typedef boost::icl::interval_map<pos_type, Allocation> icl_type;
  static Allocation npos_;

private:
  std::map<AddressSpace, icl_type> addrSpaceAllocs_;
  std::mutex access_mutex_;

public:
  size_t size() { return addrSpaceAllocs_.size(); }
  Allocation &end() {
    npos_ = Allocation();
    return npos_;
  }

  /*! \brief Lookup allocation that contains pos, size.
   */
  Allocation &find(uintptr_t pos, size_t size);

  /*! \brief Lookup allocation that contains pos, size, and address space.
   */
  Allocation &find(uintptr_t pos, size_t size, AddressSpace &as);
  /*! \brief Lookup allocation containing pos and address space
   */
  Allocation &find(uintptr_t pos, AddressSpace &as) { return find(pos, 1, as); }
  /*! \brief Lookup allocation starting at pos in address space
   */
  Allocation &find_exact(uintptr_t pos, const AddressSpace &as);

  Allocation &new_allocation(uintptr_t pos, size_t size, const AddressSpace &as,
                             const cprof::model::Memory &am,
                             const cprof::model::Location &al);

  size_t free(uintptr_t pos, const AddressSpace &as);

  Allocations() {}
  ~Allocations() { logging::err() << "DEBU: Allocations dtor\n"; }
};

} // namespace cprof
#endif
