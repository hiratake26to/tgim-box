#include "common.hpp"
#include "../schedule/schedule.hpp"

namespace tgim {

// associated event
// premitive event
// - Time
// - Sig

Time::Time(double t): value(t) {}
string Time::ToString() const {
  std::stringstream ss;
  ss << "Time{" << value << "}";
  return ss.str();
}
Time& Time::operator+=(const Time& rhs) {
  *this = *this + rhs;
  return *this;
}
Time Time::operator+(const Time& rhs) const {
  return Time { value + rhs.value };
}
bool Time::operator<(const Time& rhs) const {
  return value < rhs.value;
}
bool Time::operator> (const Time& rhs) const { return rhs < *this; }
bool Time::operator<=(const Time& rhs) const { return !(*this > rhs); }
bool Time::operator>=(const Time& rhs) const { return !(*this < rhs); }
bool Time::operator==(const Time& rhs) const {
  return value == rhs.value;
}
bool Time::operator!=(const Time& rhs) const { return !(*this == rhs); }

}
