#pragma once

#include <optional>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <any>
#include <variant>
#include <nlohmann/json.hpp>

using std::optional;
using std::string;
using std::vector;
using std::list;
using std::pair;
using std::map;
using std::any;
using json = nlohmann::json;

#include "event.hpp"
#include "action.hpp"

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
