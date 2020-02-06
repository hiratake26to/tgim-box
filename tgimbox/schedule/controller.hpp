/*
The MIT License

Copyright 2019 hiratake26to

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
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
