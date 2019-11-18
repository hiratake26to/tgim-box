#pragma once

#include <optional>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <any>
#include <variant>
#include <nlohmann/json.hpp>

// prototype for action class in task class
struct Schedule;
struct ScheduleRef: public std::reference_wrapper<Schedule> {
  bool operator==(const ScheduleRef& rhs) const {
    return true;
  }
  bool operator!=(const ScheduleRef& rhs) const { return !(*this==rhs); }
};

#include "task/task.hpp"

using std::optional;
using std::string;
using std::vector;
using std::list;
using std::pair;
using std::map;
using std::any;
using json = nlohmann::json;

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

#include "controller.hpp"
