#pragma once

#include <optional>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <any>
#include <variant>
#include <nlohmann/json.hpp>

#include "Task/Task.hpp"

using std::optional;
using std::string;
using std::vector;
using std::list;
using std::pair;
using std::map;
using std::any;
using json = nlohmann::json;

struct Task;
struct Schedule;

struct Schedule {
  list<Task> tbl;
  list<Task> sigtbl;
  bool operator==(const Schedule& rhs) const; 
  bool operator!=(const Schedule& rhs) const; 
  Schedule ConcatSigtable(const Schedule& rhs) const; 
  string ToString(int l=0) const; 
};

using ActionSpecifier=std::variant<PrimitiveAction, Sig, Schedule>;
string ActionToString(const ActionSpecifier& act, int l=0);

struct Task {
  Event evt;
  ActionSpecifier action;
  string ToString(int l=0) const;
  bool operator==(const Task& rhs) const; 
  bool operator!=(const Task& rhs) const; 
};

#include "ScheduleController.hpp"
