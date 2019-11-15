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
#include "../schedule.hpp"

using std::optional;
using std::string;
using std::vector;
using std::list;
using std::pair;
using std::map;
using std::any;
using json = nlohmann::json;

// prototype
struct PrimitiveAction;

///
// Action
//
using Action=std::variant< PrimitiveAction, Sig, ScheduleRef>;
string ActionToString(const Action& act, int l=0);

///
// PrimitiveAction
//
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

