#pragma once

#include <optional>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <any>
#include <variant>
#include <nlohmann/json.hpp>

#include "event.hpp"
#include "action.hpp"
#include "signal.hpp"
#include "scheduleref.hpp"

namespace tgim {

using std::optional;
using std::string;
using std::vector;
using std::list;
using std::pair;
using std::map;
using std::any;
using json = nlohmann::json;

///
// Task
//

struct Task {
  Event evt;
  Action action;

  string ToString(int l=0) const;
  bool operator==(const Task& rhs) const; 
  bool operator!=(const Task& rhs) const; 
};

}
