#pragma once

#include <optional>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <any>
#include <variant>
#include <nlohmann/json.hpp>

#include "Schedule.hpp"

using std::optional;
using std::string;
using std::vector;
using std::list;
using std::pair;
using std::map;
using std::any;
using json = nlohmann::json;

// prototype
struct ScheduleControllBlock;

struct ScheduleRefBox {
  Schedule& value;
  optional<std::reference_wrapper<Schedule>> super;
  static ScheduleControllBlock CreateSCB(ScheduleRefBox srb, optional<Event> evt_last);
  ScheduleControllBlock SCB(optional<Event> evt_last);
  optional<ScheduleControllBlock> AddTask(Event evt, ActionSpecifier action);
  vector<ScheduleControllBlock> AddTask(EventSpecifer es, ActionSpecifier action);
  Schedule GetSchedule() const;
};
struct ScheduleControllBlock {
  ScheduleRefBox srb; // reference to the instance of schedule table
  optional<Event> evt_last; // last pushed event
  vector<Event> evts; // accumulate OR events
  ScheduleControllBlock At(EventSpecifer es); 
  ScheduleControllBlock Aft();
  ScheduleControllBlock EndAft(); 
  ScheduleControllBlock Do(string act_type, json param); 
  ScheduleControllBlock Do(Sig sig); 
  ScheduleControllBlock Do(Schedule sdl); 
  ScheduleControllBlock Sdl(const std::function<void(ScheduleControllBlock&)>& cb); 
};
