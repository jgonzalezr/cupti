#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "cprof/allocations.hpp"
#include "cprof/profiler.hpp"

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;
using cprof::model::Location;
using cprof::model::Memory;

Allocations::value_type Allocations::find(uintptr_t pos, size_t size) {
  assert(pos && "No allocations at null pointer");

  std::vector<Allocations::value_type> matches;
  std::lock_guard<std::mutex> guard(access_mutex_);

  for (const auto &kv : addressSpaceAllocations_) {
    const auto &addressSpace = kv.first;
    const auto &allocations = kv.second;

    const auto &searchInterval =
        interval<uintptr_t>::right_open(pos, pos + size);
    const auto &ai = allocations.find(searchInterval);
    if (ai != allocations.end()) {
      matches.push_back(ai->second);
    }
  }

  if (matches.size() == 1) {
    return matches[0];
  } else if (matches.empty()) {
    return nullptr;
  } else { // FIXME for now, return most recent. Issue 11. Should be fused in
           // allocation creation?
    assert(0 && "Found allocation in multiple address spaces");
  }
}

Allocations::value_type Allocations::find(uintptr_t pos, size_t size,
                                          const AddressSpace &as) {
  assert(pos && "No allocations at null pointer");
  std::lock_guard<std::mutex> guard(access_mutex_);
  const auto &allocationsIter = addressSpaceAllocations_.find(as);
  if (allocationsIter != addressSpaceAllocations_.end()) {
    const auto &allocations = allocationsIter->second;

    auto searchInterval = interval<uintptr_t>::right_open(pos, pos + size);
    const auto &ai = allocations.find(searchInterval);
    if (ai != allocations.end()) {
      return ai->second;
    } else {
      return nullptr;
    }
  } else {
    return nullptr;
  }
}

Allocations::value_type Allocations::find_exact(uintptr_t pos,
                                                const AddressSpace &as) {
  assert(pos && "No allocations at null pointer");
  std::lock_guard<std::mutex> guard(access_mutex_);
  const auto &allocationsIter = addressSpaceAllocations_.find(as);
  if (allocationsIter != addressSpaceAllocations_.end()) {
    const auto &allocations = allocationsIter->second;

    const auto &ai = allocations.find(pos);
    if (ai != allocations.end()) {
      if (ai->second->pos() == pos)
        return ai->second;
    } else {
      return nullptr;
    }
  } else {
    return nullptr;
  }
}

Allocations::value_type Allocations::new_allocation(uintptr_t pos, size_t size,
                                                    const AddressSpace &as,
                                                    const Memory &am,
                                                    const Location &al) {
  auto val = value_type(new AllocationRecord(pos, size, as, am, al));

  if (val->size() == 0) {
    cprof::err() << "WARN: creating size 0 allocation" << std::endl;
  }

  logging::atomic_out(val->json());

  {
    std::lock_guard<std::mutex> guard(access_mutex_);
    const auto &i = interval<uintptr_t>::right_open(pos, pos + size);
    addressSpaceAllocations_[as].insert(std::make_pair(i, val));
  }
  return val;
}

size_t Allocations::free(uintptr_t pos, const AddressSpace &as) {
  auto i = find_exact(pos, as);
  if (i->freed()) {
    cprof::err() << "WARN: allocation " << i->pos() << " double-free?"
                 << std::endl;
  }
  if (i) {
    i->mark_free();
    return 1;
  }
  assert(0 && "Expecting to erase an allocation.");
  return 0;
}