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

// prototype for action class in task class
struct Schedule;
struct ScheduleRef: public std::reference_wrapper<Schedule> {
  bool operator==(const ScheduleRef& rhs) const {
    return true;
  }
  bool operator!=(const ScheduleRef& rhs) const { return !(*this==rhs); }
};

}
