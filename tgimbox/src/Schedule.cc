#include "common.hpp"
#include "../schedule/schedule.hpp"

namespace tgim {

//////////////////////////////////////////////////
// ScheduleRefBox

ScheduleControllBlock
ScheduleRefBox::CreateSCB(ScheduleRefBox srb, optional<Event> evt_last)
{
  //return {srb, evt_last, {}};
  return {{srb}, {}};
}

ScheduleControllBlock
ScheduleRefBox::SCB(optional<Event> evt_last) const
{
  return CreateSCB(*this, evt_last);
}

optional<ScheduleRefBox>
ScheduleRefBox::AddTask(Event evt, Action action)
{
  Task* p_last = nullptr;
  if (std::get_if<Sig>(&evt.value())) {
    this->value.get().sigtbl.push_back(Task{evt, action});
    p_last = &this->value.get().sigtbl.back();
  } else {
    this->value.get().tbl.push_back(Task{evt, action});
    p_last = &this->value.get().tbl.back();
  }
  if (p_last == nullptr) throw std::logic_error("p_last is nullptr!");

  // Action type PriAct or Sig or Schedule
  if (ScheduleRef *p_sdl_ref = std::get_if<ScheduleRef>(&p_last->action)) {
    return ScheduleRefBox{p_sdl_ref->get()}; // return SCB
  }

  return {};
}
// [Event] -> [Action] -> SCB
optional<vector<ScheduleRefBox>>
ScheduleRefBox::AddTask(vector<Event> evts, Action action)
{
  vector<ScheduleRefBox> ret;
  for (const Event& evt : evts) {
    if (auto result = this->AddTask(evt, action)) {
      ret.push_back(result.value());
    }
  }
  
  if (ret.size() == 0) return {};
  return ret;
}

Schedule ScheduleRefBox::GetSchedule() const {
  return value;
}

//////////////////////////////////////////////////
// ScheduleControllBlock

ScheduleControllBlock
ScheduleControllBlock::At(EventSpecifer es) {
  vector<Event> v = es.value();
  //evt_last = v[v.size()-1];
  this->evts.insert(this->evts.end(), RANGE(v));
  return *this;
}
ScheduleControllBlock
ScheduleControllBlock::Aft() {
  //if (not evt_last) {
  if (evts.size() == 0) {
    throw std::runtime_error("Aft failed, dut to no exist befor event");
  }

  ScheduleControllBlock nest_scb;
  for (auto& srb : this->srbs) {
    // append child into each `schedule.children` in srb
    srb.value.get().children.push_back(Schedule{{},{},srb.value,{}});
    // get the child's reference
    ScheduleRef sr = ScheduleRef{srb.value.get().children.back()};

      //srb.AddTask(evt_last.value_or(Event{0}), sr).value();
    if (auto result_scb = srb.AddTask(evts, sr)) {
      nest_scb.srbs.insert( nest_scb.srbs.end(), RANGE(result_scb.value()));
    }
  }
  return nest_scb;
}
ScheduleControllBlock 
ScheduleControllBlock::EndAft() {
  if (this->srbs.size() == 0) {
    throw std::logic_error("this is invalid SCB");
  }
  if (not this->srbs[0].value.get().parent) {
    throw std::runtime_error("EndAft failed, dut to no exist parent schedule control block");
  }

  return ScheduleControllBlock{
    {ScheduleRefBox{this->srbs[0].value.get().parent.value()}}
  };
}
ScheduleControllBlock 
ScheduleControllBlock::Do(string act_type, json param) {
  if (this->evts.size() == 0) this->At(0);
  for (const auto& e : this->evts) {
    for (auto&& srb : this->srbs) {
      srb.AddTask(e, PrimitiveAction{act_type, param});
    }
  }
  return {this->srbs, {}}; // reset evts
}
ScheduleControllBlock 
ScheduleControllBlock::Do(Sig sig) {
  if (this->evts.size() == 0) this->At(0);
  for (const auto& e : this->evts) {
    for (auto&& srb : this->srbs) {
      srb.AddTask(e, sig);
    }
  }
  return {this->srbs, {}}; // reset evts
}
ScheduleControllBlock 
ScheduleControllBlock::Do(Schedule sdl) {
  for (auto&& srb : this->srbs) {
    srb.value.get().children.push_back(sdl);

    ScheduleRef sr = ScheduleRef{srb.value.get().children.back()};

    if (this->evts.size() == 0) this->At(0);
    for (const auto& e : this->evts) {
      srb.AddTask(e, sr);
    }
  }

  return {this->srbs, {}}; // reset evts
}
ScheduleControllBlock 
ScheduleControllBlock::Sdl(const std::function<void(ScheduleControllBlock&)>& cb)
{
  if (this->evts.size() == 0) this->At(Time{0});
  auto next_scb = this->Aft();
  cb(next_scb);
  return next_scb.EndAft();
}
ScheduleControllBlock 
ScheduleControllBlock::Sdl(const Schedule& sdl)
{
  for (auto&& srb : this->srbs) {
    srb.value.get() = srb.value.get().SimplyConcat(sdl);
  }
  return *this;
}

//////////////////////////////////////////////////
// Schedule
//

// cannot use vector dut to possible invalid reference
//https://ja.cppreference.com/w/cpp/container/list
//list<Task> Schedule::tbl;
//list<Task> Schedule::sigtbl;

Schedule Schedule::SimplyConcat(const Schedule& rhs) const {
  Schedule ret = *this;
  // concatinate Schedule::tbl
  for (const auto& task : rhs.tbl) {
    ret.tbl.push_back(task);
  }
  for (const auto& sigtask : rhs.sigtbl) {
    ret.sigtbl.push_back(sigtask);
  }
  return ret;
}
// no overwrite sigtbl when this has already sigtbl's signal.
Schedule Schedule::SetSuperSigtbl(const Schedule& rhs) const {
  Schedule ret = *this;
  for (const auto& sigtask : rhs.sigtbl) {
    // continue if it has already
    if (this-> sigtbl.end() !=
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

}
