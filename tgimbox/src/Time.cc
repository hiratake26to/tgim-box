/*
The MIT License

Copyright 2019 hiratake26to

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
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
