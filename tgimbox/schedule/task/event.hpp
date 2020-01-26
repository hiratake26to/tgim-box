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
