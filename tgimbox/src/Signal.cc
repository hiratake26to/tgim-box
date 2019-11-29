#include "common.hpp"
#include "../schedule/schedule.hpp"

namespace tgim {

string Sig::ToString() const {
  std::stringstream ss;
  ss << "Sig{\"" << value << "\"}";
  return ss.str();
}
bool Sig::operator==(const Sig& rhs) const {
  return value == rhs.value;
}
bool Sig::operator!=(const Sig& rhs) const { return !(*this == rhs); }
bool Sig::operator<(const Sig& rhs) const {
  return value < rhs.value;
}
bool Sig::operator> (const Sig& rhs) const { return rhs < *this; }
bool Sig::operator<=(const Sig& rhs) const { return !(*this > rhs); }
bool Sig::operator>=(const Sig& rhs) const { return !(*this < rhs); }

}
