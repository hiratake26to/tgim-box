#include "common.hpp"

//////////////////////////////////////////////////
// ScheduleRefBox
//

ScheduleControllBlock
ScheduleRefBox::CreateSCB(ScheduleRefBox srb, optional<Event> evt_last)
{
  return {srb, evt_last, {}};
}

ScheduleControllBlock
ScheduleRefBox::SCB(optional<Event> evt_last)
{
  return CreateSCB(*this, evt_last);
}

optional<ScheduleControllBlock>
ScheduleRefBox::AddTask(Event evt, ActionSpecifier action)
{
  Task* p_last = nullptr;
  if (std::get_if<Sig>(&evt.value())) {
    this->value.sigtbl.push_back(Task{evt, action});
    p_last = &this->value.sigtbl.back();
  } else {
    this->value.tbl.push_back(Task{evt, action});
    p_last = &this->value.tbl.back();
  }
  if (p_last == nullptr) throw std::logic_error("p_last is nullptr!");

  // Action type PriAct or Sig or Schedule
  if (Schedule *p_sdl = std::get_if<Schedule>(&p_last->action)) {
    return CreateSCB(ScheduleRefBox{*p_sdl, this->value}, p_last->evt); // return SCB
  }

  return {};
}
// [Event] -> [Action] -> [SCB]
vector<ScheduleControllBlock>
ScheduleRefBox::AddTask(EventSpecifer es, ActionSpecifier action)
{
  vector<ScheduleControllBlock> scbs;
  vector<Event> evts = es.value();
  for (const Event& evt : evts) {
    Task* p_last = nullptr; // a task that added last to hold for CreateSCB
    if (std::get_if<Sig>(&evt.value())) {
      this->value.sigtbl.push_back(Task{evt, action});
      p_last = &this->value.sigtbl.back();
    } else {
      this->value.tbl.push_back(Task{evt, action});
      p_last = &this->value.tbl.back();
    }
    if (p_last == nullptr) throw std::logic_error("p_last is nullptr!");

    // Action type PriAct or Sig or Schedule
    if (Schedule *p_sdl = std::get_if<Schedule>(&p_last->action)) {
      scbs.push_back(
          //CreateSCB(ScheduleRefBox{*p_sdl, this->value}, p_last->evt) // add SCB
          CreateSCB(ScheduleRefBox{*p_sdl, this->value}, {}) // add SCB
          );
    }
  }

  return scbs;
}

Schedule ScheduleRefBox::GetSchedule() const {
  return value;
}

//////////////////////////////////////////////////
// ScheduleControllBlock
//

ScheduleControllBlock
ScheduleControllBlock::At(EventSpecifer es) {
  vector<Event> v = es.value();
  evt_last = v[v.size()-1];
  this->evts.insert(this->evts.end(), RANGE(v));
  return *this;
}
ScheduleControllBlock
ScheduleControllBlock::Aft() {
  if (not evt_last) {
    throw std::runtime_error("Aft failed, dut to no exist befor event");
  }

  ScheduleControllBlock nest_scb =
    this->srb.AddTask(evt_last.value_or(Event{0}), Schedule{}).value();
  return nest_scb;
}
ScheduleControllBlock 
ScheduleControllBlock::EndAft() {
  if (not srb.super) {
    throw std::runtime_error("EndAft failed, dut to no exist super schedule control block");
  }

  return {this->srb.super.value()};
}
ScheduleControllBlock 
ScheduleControllBlock::Do(string act_type, json param) {
  if (this->evts.size() == 0) this->At(0);
  for (const auto& e : this->evts) {
    srb.AddTask(e, PrimitiveAction{act_type, param});
  }
  return {this->srb, this->evt_last};
  //return {this->srb}; //reset
}
ScheduleControllBlock 
ScheduleControllBlock::Do(Sig sig) {
  if (this->evts.size() == 0) this->At(0);
  for (const auto& e : this->evts) {
    srb.AddTask(e, sig);
  }
  return {this->srb, this->evt_last};
  //return {this->srb}; //reset
}
ScheduleControllBlock 
ScheduleControllBlock::Do(Schedule sdl) {
  if (this->evts.size() == 0) this->At(0);
  for (const auto& e : this->evts) {
    srb.AddTask(e, sdl);
  }
  return {this->srb, this->evt_last};
  //return {this->srb}; //reset
}
ScheduleControllBlock 
ScheduleControllBlock::Sdl(const std::function<void(ScheduleControllBlock&)>& cb)
{
  if (this->evts.size() == 0) this->At(0);
  // from a new Schedule into a ScheduleRefBox,
  // then create a SCB for nested-schedule 
  // and call the callback with the SCB
  Schedule nest_sdl;
  //auto scb = ScheduleRefBox{temp}.SCB(evt_last); // TODO better calling with evt_last?
  auto scb = ScheduleRefBox{nest_sdl}.SCB({});
  cb(scb);
  // nested-schedule into this 
  return this->Do(nest_sdl);
}

//////////////////////////////////////////////////
// Schedule
//

// cannot use vector dut to possible invalid reference
//https://ja.cppreference.com/w/cpp/container/list
//list<Task> Schedule::tbl;
//list<Task> Schedule::sigtbl;

// no overwrite sigtbl when this has already sigtbl's signal.
Schedule Schedule::ConcatSigtable(const Schedule& rhs) const {
  Schedule ret = *this;
  for (const auto& sigtask : rhs.sigtbl) {
    // continue if it has already
    if (this-> sigtbl.end() 
        != // find condition
        std::find_if(RANGE(this->sigtbl),
          [&sigtask](auto task){ return task.evt == sigtask.evt; }
        ))
    {
      continue;
    }
    ret.sigtbl.push_back(sigtask);
  }
  return std::move(ret);
}

string Schedule::ToString(int l) const {
  std::stringstream ss;
  ss << string(l,' ') << "Schedule{\n";
  ss << string(l,' ') << "tbl[\n";
  for (const auto& task : this->tbl) {
    ss << string(l,' ') << "  " << task.ToString(l+2) << ",\n";
  }
  ss << string(l,' ') << "],\n";
  ss << string(l,' ') << "sigtbl[\n";
  for (const auto& task : this->sigtbl) {
    ss << string(l,' ') << "  " << task.ToString(l+2) << ",\n";
  }
  ss << string(l,' ') << "]\n";
  ss << string(l,' ') << "}";
  return ss.str();
}

bool Schedule::operator==(const Schedule& rhs) const {
  return (this->tbl == rhs.tbl and this->sigtbl == rhs.sigtbl);
}
bool Schedule::operator!=(const Schedule& rhs) const { return !(*this==rhs); }
