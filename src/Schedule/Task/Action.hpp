#pragma once

#include <optional>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <any>
#include <variant>
#include <nlohmann/json.hpp>

#include "Signal.hpp"
#include "../Schedule.hpp"

using std::optional;
using std::string;
using std::vector;
using std::list;
using std::pair;
using std::map;
using std::any;
using json = nlohmann::json;

// Action
struct PrimitiveAction {
  string type; // ActionType (NSOM-AppType)
  json param;
  PrimitiveAction(string type, json param);
  string ToString() const; 
  bool operator==(const PrimitiveAction& rhs) const; 
  bool operator!=(const PrimitiveAction& rhs) const; 
  bool operator<(const PrimitiveAction& rhs) const; 
  bool operator> (const PrimitiveAction& rhs) const; 
  bool operator<=(const PrimitiveAction& rhs) const; 
  bool operator>=(const PrimitiveAction& rhs) const; 
};

