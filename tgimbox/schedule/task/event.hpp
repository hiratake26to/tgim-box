/*
The MIT License

Copyright 2019 hiratake26to

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once

#include <optional>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <any>
#include <variant>
#include <nlohmann/json.hpp>

#include "signal.hpp"

namespace tgim {

using std::optional;
using std::string;
using std::vector;
using std::list;
using std::pair;
using std::map;
using std::any;
using json = nlohmann::json;

// Time
struct Time {
  double value;
  Time(double t);
  Time& operator+=(const Time& rhs); 
  Time operator+(const Time& rhs) const; 
  bool operator<(const Time& rhs) const; 
  bool operator> (const Time& rhs) const; 
  bool operator<=(const Time& rhs) const; 
  bool operator>=(const Time& rhs) const; 
  bool operator==(const Time& rhs) const; 
  bool operator!=(const Time& rhs) const; 
  string ToString() const; 
};

// Range
template<typename T>
struct Range {
  T start;
  T stop;
  T interval;
  Range(T start, T stop, T interval): start(start), stop(stop), interval(interval) {
    if (not (start < stop)) {
      throw std::runtime_error("range construction error: could not be `start < stop`");
    }
    if (interval <= 0) {
      throw std::runtime_error("range construction error: could not specified interval to less than equal 0.");
    }
  }
  Range(T start, T stop): Range(start, stop, 1) {}
};
template struct Range<Time>;

// Event
class Event {
public:
  using Type = std::variant<Time,Sig>;
  Event(Time value); 
  Event(Sig value); 
  const Type& value() const noexcept; 
  string ToString() const; 
  Event operator+(const Event& rhs) const; 
  Event operator+=(const Event& rhs); 
  bool operator==(const Event& rhs) const; 
  bool operator!=(const Event& rhs) const; 
  bool operator<(const Event& rhs) const; 
  bool operator> (const Event& rhs) const; 
  bool operator<=(const Event& rhs) const; 
  bool operator>=(const Event& rhs) const; 
private:
  Type value_;
};
class EventSpecifer {
  vector<Event> v_; // OR
public:
  EventSpecifer(); 
  EventSpecifer(Event evt); 
  EventSpecifer(Time time); 
  EventSpecifer(double time); 
  EventSpecifer(Sig id); 
  template<typename T> EventSpecifer(Range<T> range) {
    if (not (range.start <= range.stop)) throw std::logic_error(
        "Range of event is invalid value "
        "that it is not `start` less than equal `stop`."
        );
    for (T cur = range.start; cur < range.stop; cur += range.interval) {
      this->v_.push_back(Event{cur});
    }
  }
  vector<Event> value() const; 
};

}
