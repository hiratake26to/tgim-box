#pragma once

#include "common.h"

#include "task/task.hpp"
#include "controller.hpp"

namespace tgim {

///
// Schedule
//
struct Schedule {
  list<Task> tbl;
  list<Task> sigtbl;
  /// optional parent schedule as reference
  optional<ScheduleRef> parent;
  list<Schedule> children;

  Schedule SimplyConcat(const Schedule& rhs) const;
  Schedule SetSuperSigtbl(const Schedule& rhs) const; 
  string ToString(int l=0) const; 
  bool operator==(const Schedule& rhs) const; 
  bool operator!=(const Schedule& rhs) const; 
};

}
