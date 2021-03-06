#include "address_space.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <sstream>

using boost::property_tree::ptree;
using boost::property_tree::write_json;

static std::string to_string(const AddressSpace::Type &t) {
  switch (t) {
  case AddressSpace::Type::Host:
    return "host";
  case AddressSpace::Type::Cuda:
    return "cuda";
  case AddressSpace::Type::Unknown:
    return "unknown";
  default:
    assert(0 && "Unhandled AddressSpace::Type");
  }
}

std::string AddressSpace::json() const {
  ptree pt;
  pt.put("type", to_string(type_));
  std::ostringstream buf;
  write_json(buf, pt, false);
  return buf.str();
}

bool AddressSpace::maybe_equal(const AddressSpace &other) const {
  assert(is_valid());
  return other == *this || is_unknown() || other.is_unknown();
}