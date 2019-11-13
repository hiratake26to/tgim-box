#include "common.hpp"

//////////////////////////////////////////////////
// Event
// (premitive) Event [ Time(int) | Sig(int) ]

//using Event::Type = std::variant<Time,Sig>;
//Type Event::value_;
Event::Event(Time value): value_(value) {}
Event::Event(Sig value): value_(value) {}
const Event::Type& Event::value() const noexcept { return value_; }
Event Event::operator+(const Event& rhs) const {
  if (auto&& lhs_val = std::get_if<Time>(&this->value_))
  if (auto&& rhs_val = std::get_if<Time>(&rhs.value_)) {
    return {*lhs_val + *rhs_val};
  }
  throw std::logic_error("exception: operator `+` use only to Time");
}
Event Event::operator+=(const Event& rhs) {
  *this = *this + rhs;
  return *this;
}
bool Event::operator==(const Event& rhs) const {
  return this->value_==rhs.value_;
}
bool Event::operator!=(const Event& rhs) const { return !(*this==rhs); }
string Event::ToString() const {
  if (auto&& time = std::get_if<Time>(&this->value_)) {
    return time->ToString();
  }
  if (auto&& sig = std::get_if<Sig>(&this->value_)) {
    return sig->ToString();
  }
  throw std::logic_error("exception: no support ToString convertion!");
}
bool Event::operator<(const Event& rhs) const {
  return this->value_ < rhs.value_;
}
bool Event::operator> (const Event& rhs) const { return rhs < *this; }
bool Event::operator<=(const Event& rhs) const { return !(*this > rhs); }
bool Event::operator>=(const Event& rhs) const { return !(*this < rhs); }

//////////////////////////////////////////////////
// EventSpecifer 
// this is ether Time, Range<Time>
//

EventSpecifer::EventSpecifer() {
  // immediately event
  v_.push_back(Event{Time{0}});
}
EventSpecifer::EventSpecifer(Event evt) {
  v_.push_back(evt);
}
EventSpecifer::EventSpecifer(Time time) {
  v_.push_back(Event{time});
}
EventSpecifer::EventSpecifer(int time) {
  v_.push_back(Event{Time{time}});
}
EventSpecifer::EventSpecifer(Sig id) {
  v_.push_back(Event{id});
}
vector<Event> EventSpecifer::value() const {
  return v_;
}

