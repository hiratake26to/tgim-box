#include "common.hpp"
#include "../schedule/schedule.hpp"

namespace tgim {

// Task is move-only and use only via reference.
string Task::ToString(int l) const {
  std::stringstream ss;
  ss << "Task{evt="
    << evt.ToString()
    << ",act="
    << ActionToString(action, l+2)
    << "}";
  return ss.str();
};

bool Task::operator==(const Task& rhs) const {
  return (
      evt == rhs.evt &&
      action == rhs.action
      );
}
bool Task::operator!=(const Task& rhs) const { return !(*this == rhs); }

}
