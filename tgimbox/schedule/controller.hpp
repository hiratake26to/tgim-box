#pragma once

#include "common.h"
#include "schedule.hpp"

namespace tgim {

// prototype
struct ScheduleControllBlock;

///
// ScheduleRefBox
// This contain a box as reference.
// Note. Box is container of reference(or pointer),
//       isn't Network Box.
struct ScheduleRefBox {
  /// reference to the instance of Schedule
  ScheduleRef value;

  ///
  // CreateSCB: is utility function.
  // return SCB(ScheduleControllBlock) from ScheduleRefBox.
  // optional `evt_last` put into the last event of setting.
  static ScheduleControllBlock CreateSCB(ScheduleRefBox srb, optional<Event> evt_last);
  ///
  // SCB: return SCB of self.
  ScheduleControllBlock SCB(optional<Event> evt_last) const;
  ///
  // AddTask: add a task by set of one event and one action.
  // return SCB of nested-schedule, if the action added is schedule.
  // Event -> Action -> Optional SCB
  optional<ScheduleRefBox> AddTask(Event evt, Action action);
  ///
  // AddTask: add tasks by set of multi event and one action.
  // return SCBs of nested-schedules by the action.
  // [Event] -> Action -> [SCB]
  optional<vector<ScheduleRefBox>> AddTask(vector<Event> evts, Action action);
  ///
  // GetSchedule: return a schedule that is a copy self reference.
  Schedule GetSchedule() const;
};

///
// ScheduleControllBlock (SCB)
// SCB provide functions to controll the schedule
struct ScheduleControllBlock {
  /// reference to the instance of schedule table
  vector<ScheduleRefBox> srbs;
  /// last pushed event
  //optional<Event> evt_last;
  /// accumulate OR events
  vector<Event> evts;

  ///
  // specifer timing
  ScheduleControllBlock At(EventSpecifer es);
  ///
  // return a SCB for the nested schedule
  ScheduleControllBlock Aft();
  ///
  // return a SCB of the parent of the SCB at present.
  ScheduleControllBlock EndAft(); 
  ///
  // specifer what to do
  ScheduleControllBlock Do(string act_type, json param); 
  ///
  // specifer signal sending
  ScheduleControllBlock Do(Sig sig); 
  ///
  // specifer schedule
  ScheduleControllBlock Do(Schedule sdl); 
  ///
  // schedule nest and controll shcedule callback
  ScheduleControllBlock Sdl(const std::function<void(ScheduleControllBlock&)>& cb); 
  ///
  // concatinate the shcedule to this
  ScheduleControllBlock Sdl(const Schedule& sdl);
};

}