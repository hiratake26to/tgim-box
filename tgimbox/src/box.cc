#include "common.hpp"
#include "../schedule/schedule.hpp"
#include "../box/box.hpp"
#include "../box/order.hpp"

namespace tgim {

// find tasks that have the event`evt` as a action.
// T: parent task.aciton type
// U: type of variant
template<typename T, typename U>
vector<Task> FindTasksActionAs(const vector<Task>& tasks, const U& value) {
  vector<Task> ret;
  if (auto&& evt_act = std::get_if<T>(&value)) {
    for (const auto& task : tasks)
    //if (auto&& task_act = std::get_if<T>(&task.action.value())) {
    if (auto&& task_act = std::get_if<T>(&task.action)) {
      //cout << "[DEBUG] compare: "
      //  << ActionToString(*evt_act) 
      //  << " and "
      //  << ActionToString(task.action)
      //  << endl;
      if (*task_act != *evt_act) {
        continue;
      } else {
        ret.push_back(task);
        //cout << "[DEBUG] add: " << task.ToString() << endl;
      }
    }
  } else {
    std::stringstream ss;
    ss << "exception: faild to find tasks due to get value as type of Sig";
    throw std::logic_error(ss.str());
  }

  return ret;
}


//////////////////////////////////////////////////
/// Box
Box::Box(string name, string type): name_(name), type_(type) {
  // initialization
  point_ = {};
}

// [TODO]
// 現実装では, 名前の変更は接続処理の前でなければならない.
// 名前はポインタのように扱われるため, 接続後に名前を変更した場合, 動作は未定義.
//
// 接続後の名前の変更を禁止にするアイディアはどうだろうか.
Box& Box::SetName(string name) {
  this->name_ = name;
  return *this;
}

Box& Box::SetType(string type) {
  this->type_ = type;
  return *this;
}

Box& Box::SetPoint(Point pt) {
  this->point_ = pt;
  return *this;
}

string Box::GetName() const {
  return this->name_;
}

string Box::GetType() const {
  return this->type_;
}

Point Box::GetPoint() const {
  return this->point_;
}

Box Box::Fork(string name) const {
  Box ret = *this;
  ret.SetName(name);
  return ret;
}

Box Box::Fork(string name, string type) const {
  Box ret = *this;
  ret.SetName(name);
  ret.SetType(type);
  return ret;
}

// Box::At method is bootstrap of ScheduleControllBlock
ScheduleControllBlock Box::At(EventSpecifer es) {
  return ScheduleRefBox{this->schedule_}.SCB({}).At(es);
}
ScheduleControllBlock Box::Sdl(const Schedule& sdl) {
  return ScheduleRefBox{this->schedule_}.SCB({}).Sdl(sdl);
}
ScheduleControllBlock Box::Sdl2() {
  return ScheduleRefBox{this->schedule_}.SCB({});
}

vector<Event> Box::SearchPrimitiveEvents(const vector<Task>& sdl, const Task& task, vector<Task>& breadcrumb) {
  breadcrumb.push_back(task);
  //cout << "[DEBUG] search from: " << task.ToString() << endl;
  // if task.action is type of Time, resolve already!
  if (std::get_if<Time>(&task.evt.value())) {
    //cout << "[DEBUG] find!: " << task.ToString() << endl;
    return {task.evt};
  }

  // resolve Event(parent would have Action) from parent Task recursive
  vector<Event> ret;
  for (const auto& parent_task : FindTasksActionAs<Sig>(sdl, task.evt.value())) {
    //cout << "[DEBUG] for parent task: " << parent_task.ToString() << endl;
    // skip if parent task contains in breadcrumb
    if (std::find(RANGE(breadcrumb), parent_task) != std::end(breadcrumb)) {
      //cout << "[DEBUG] skip: " << parent_task.ToString() << endl;
      continue;
    }
    auto&& evts = SearchPrimitiveEvents(sdl, parent_task, breadcrumb);
    ret.insert(ret.end(), RANGE(evts));
  }

  // delete duplicate events
  {
    std::sort(RANGE(ret));
    auto last = std::unique(RANGE(ret));
    ret.erase(last, ret.end());
  }

  //cout << "[DEBUG] current breadcrumb: " << breadcrumb.size() << endl;
  //cout << "[DEBUG] signal of task resolved: " << ret.size() << endl;

  return ret;
}
vector<Event> Box::SearchPrimitiveEvents(const vector<Task>& sdl, const Task& task) const {
  vector<Task> task_crumb; // avoid for duplication search
  return SearchPrimitiveEvents(sdl, task, task_crumb);
}

void Box::AddTask(Event evt, Action action) {
  if (std::get_if<Sig>(&evt.value())) {
    this->schedule_.sigtbl.push_back(Task{evt, action});
  } else {
    this->schedule_.tbl.push_back(Task{evt, action});
  }
}
Schedule Box::GetSchedule() const {
  return this->schedule_;
}

string Box::DumpSchedule() const {
  std::stringstream ss;
  ss << schedule_.ToString();
  return ss.str();
}

vector<Task> Box::FlattenSchedule(const Schedule& sdl,int callid) const {
  vector<Task> ret;
  //cout << "[DEBUG!!CALL("<<callid<<")]" << endl;
  //cout << sdl.ToString() << endl;
  for (auto&& task : sdl.tbl) {
    // Primitive task
    if (std::get_if<PrimitiveAction>(&task.action)) {
      //cout << "[DEBUG] Resolve pre-act: " << task.ToString() << endl;
      ret.push_back(task);
      continue;
    }
    // task's Action with Sig
    if (auto&& ptr_act = std::get_if<Sig>(&task.action)) {
      //cout << "[DEBUG] Resolve signal for " << task.ToString() << endl;
      Schedule temp{{}, sdl.sigtbl};
      for (auto&& sigtask : sdl.sigtbl) {
        if (Event{*ptr_act} == sigtask.evt) {
          // Task is not still resolve action here
          // e.g.
          // tbl: [
          //   Task{task.evt, Sig{A}} ... target
          // ]
          // sigtbl: [
          //   Task{Sig{A}, PreAct{x}} ... resolve!
          //   Task{Sig{A}, Sig{B}} ... yet resolve.
          //   Task{Sig{A}, Schedule{...}} ... yet resolve.
          // ]
          temp.tbl.push_back(Task{task.evt, sigtask.action});
        }
      }
      const auto& result = FlattenSchedule(temp, callid+1);
      ret.insert(ret.end(), RANGE(result));
      continue;
    }
    // task in the schedule
    if (auto&& ptr = std::get_if<ScheduleRef>(&task.action)) {
      const Schedule& nested_sdl = ptr->get();
      const auto& temp = nested_sdl.SetSuperSigtbl(sdl);

      //cout << "[DEBUG] Resolve nested schedule for " << task.ToString() << endl;
      //cout << "[nested_sdl]" << endl;
      //cout << nested_sdl.ToString() << endl;
      //cout << "[sdl]" << endl;
      //cout << sdl.ToString() << endl;
      //cout << "[temp]" << endl;
      //cout << temp.ToString() << endl;

      auto&& result = FlattenSchedule(temp, callid+1);
      // result task's event add task.evt
      for (auto&& r : result) { r.evt += task.evt; }
      if (result.size() > 0) {
        ret.insert(ret.end(), RANGE(result));
      }
      continue;
    }
  }

  //cout << "[DEBUG!!EXIT("<<callid<<")]" << endl;
  //int idx = 0;
  //for (auto&& i : ret) {
  //  cout << "["<<(idx++)<<"] " << i.ToString() << endl;
  //}

  return ret;
}
vector<Task> Box::FlattenSchedule() const {
  return FlattenSchedule(this->schedule_);
}
void Box::ClearSchedule() {
  this->schedule_ = {};
}

// resolve the merged channel from a channel.
// channel_name is channel's name of a node-to-channel connection.
// 1. search channel from inner box
// 2. channel resolve to merged-channel
// 3. return merged-channl
optional<string> Box::ResolveConn(string channel_name) const {
  try {
    Channel& ch = this->FindChannel(channel_name).value();
    Port2& port = this->FindPort2(ch.port.value()).value();
    MergedChannel mch = port.mchannel.value();
    return mch.value.name; // merged channel name
  } catch(const std::bad_optional_access& e) {
    return {};
  }

}

vector<Netif> Box::GenNodeConnList(const Node& node) const {
  vector<Netif> node_conn_list = node.GenConnList();
  // connection list of node-to-channel resolve to node-to-merged-channel
  for (auto&& netif : node_conn_list) {
    netif.connect = this->ResolveConn(netif.connect).value_or(netif.connect);
  }
  return node_conn_list;
}

string Box::PrefixBox(string name) const {
    std::stringstream ss;
    ss << "_B" << name;
    return ss.str();
}
string Box::PrefixNode(string name) const {
    std::stringstream ss;
    ss << "_N" << name;
    return ss.str();
}
string Box::PrefixChannel(string name) const {
    std::stringstream ss;
    ss << "_C" << name;
    return ss.str();
}
string Box::PrefixPort(string name) const {
    std::stringstream ss;
    ss << "_P" << name;
    return ss.str();
}

// name mangling for internal nodes, channels and ports.
// type prefix;
// - Box     : B
// - Node    : N
// - Channel : C
void Box::Mangle() {
  // first, this box name mangling
  this->name_ = PrefixBox(this->name_);

  for (auto&& node : this->nodes) {
    // node name mangle
    node.name = this->name_ + PrefixNode(node.name);

    // connect_to name mangle
    // note that distination of node.connect_to must be internal box.
    for (auto&& conn : node.connect_to) {
      conn = this->name_ + PrefixChannel(conn);
    }
  }
  for (auto&& ch : this->channels) {
    // channel name mangle
    ch.name = this->name_ + PrefixChannel(ch.name);
    ch.port = (ch.port ? PrefixPort(ch.port.value()) : nullptr);
  }
  // port name mangling
  for (auto&& port : this->ports) {
    // TODO connect_to mangling correspond to channel name mangling
    port.name = PrefixPort(port.name);
    port.from_channel = this->name_ + PrefixChannel(port.from_channel);
    port.to_box = PrefixBox(port.to_box);
    port.to_box_port = PrefixPort(port.to_box_port);

  }
}

Node& Box::CreateNode(string name, string type) {
  nodes.push_back(Node{name, type});
  return FindNode(name).value();
}

Node& Box::CopyNode(string src, string dst) {
  auto&& result = FindNode(src);
  if (not result) {
    std::stringstream ss;
    ss << "could not copy node, no exist node has name of \"" << src << "\"";
    throw std::runtime_error(ss.str());
  }
  Node node = result.value();
  node.name = dst;
  nodes.push_back(node);
  return FindNode(dst).value();
}

Channel& Box::CreateChannel(string name, string type) {
  channels.push_back(Channel{name, type});
  return FindChannel(name).value();
}

Channel& Box::CopyChannel(string src, string dst) {
  auto&& result = FindChannel(src);
  if (not result) {
    std::stringstream ss;
    ss << "could not copy channel, no exist channel has name of \"" << src << "\"";
    throw std::runtime_error(ss.str());
  }
  Channel channel = result.value();
  channel.name = dst;
  channels.push_back(channel);
  return FindChannel(dst).value();
}

optional<std::reference_wrapper<Channel>> Box::FindChannel(string name) const {
  for (Channel& i : channels) {
    if (i.name == name) return i;
  }
  return {};
}

optional<std::reference_wrapper<Node>> Box::FindNodeFromType(string type) const {
  for (Node& i : nodes) {
    if (i.type == type) return i;
  }
  return {};
}
optional<std::reference_wrapper<Node>> Box::FindNode(string name) const {
  for (Node& i : nodes) {
    if (i.name == name) return i;
  }
  return {};
}

optional<std::reference_wrapper<Port2>> Box::FindPort2(string name) const {
  for (Port2& i : ports) {
    if (i.name == name) return i;
  }
  return {};
}

optional<string> Box::CheckInvalidChannel(const Channel& from_channel, const Channel& to_channel) {
  // 1対1接続であることを確認
  if (from_channel.port || to_channel.port) {
    std::stringstream ss;
    if (from_channel.port) ss << "from channel: " << from_channel.name << " is used." << endl;
    if (to_channel.port) ss << "to channel: " << to_channel.name << " is used." << endl;
    return ss.str();
  }
  return {};
}

/* attempt to automatically construct port, what happens?
void CreatePort(const string& port_name) {
  return BindPort(this->default_channel_, port_name);
}
*/
Port2& Box::CreatePort(const string& from_this_channel_name, const string& port_name) {
  Channel& from_channel = this->FindChannel(std::move(from_this_channel_name)).value();
  if (const auto& a = FindPort2(port_name)) {
    const Port2& p = a.value();
    std::stringstream ss;
    ss << "exception: the port specified `port_name` already exists." << endl;
    ss << "\tbox dump: " << ToString() << endl;
    ss << "\tport dump: " << p.ToString() << endl;
    ss << "\tcould not create to same name ports";
    throw std::logic_error(ss.str());
  }

  ports.push_back(
      Port2{port_name, from_this_channel_name, "", "", 0, 0});

  from_channel.port = port_name;

  return FindPort2(port_name).value();
}
void Box::SetPortMaxConnection(const string& port_name, int num) {
  Port2& port = FindPort2(std::move(port_name)).value();
  port.max_conn = num;
}

void Box::ConnectPort(const string& from_this_port_name, Box& to_box, const string& to_box_port_name) {
  auto&& result_from_port = this->FindPort2(from_this_port_name);
  auto&& result_to_port = to_box.FindPort2(to_box_port_name);

  if (not result_from_port || not result_to_port) {
    std::stringstream ss;
    ss << "exception: specified port is invalid." << endl;
    if (not result_from_port) {
      ss << "\tsource port \""<<from_this_port_name<<"\" is not found" << endl;
    }
    if (not result_to_port) {
      ss << "\tdistination \""<<to_box_port_name<<"\" port is not found" << endl;
    }
    throw std::logic_error(ss.str());
  }

  Port2& from_port = result_from_port.value();
  Port2& to_port = result_to_port.value();
  /* TODO
  if (auto msg = this->CheckInvalidChannel(from_channel, to_channel)) {
    std::cerr << "connection failed. object dump:" << endl;
    std::cerr << "from box: " << this->ToString() << endl;
    std::cerr << "to box: " << to_box.ToString() << endl;
    std::cerr << "checker message: " << msg.value() << endl;
    throw std::logic_error("not pass through check a channel");
  }
  */

  // FIXME multi-connecion
  if (from_port.to_box != "" or to_port.to_box != "") {
    std::stringstream ss;
    ss << "exception: port connection error, port is already used" << endl;
    if (from_port.to_box != "") {
      ss << "\tsource port is already connected" << endl;
    }
    if (to_port.to_box != "") {
      ss << "\tdistination port is already connected" << endl;
    }
    throw std::logic_error(ss.str());
  }

  from_port.to_box = to_box.GetName();
  from_port.to_box_port = to_box_port_name;
  from_port.relation = 1;
  from_port.is_parent = 0;

  to_port.to_box = this->GetName();
  to_port.to_box_port = from_this_port_name;
  to_port.relation = 1;
  to_port.is_parent = 1;

  // update current connection count
  try {
    from_port.IncCurConn();
    to_port.IncCurConn();
  } catch(const std::logic_error& e) {
    std::stringstream ss;
    ss << e.what() << endl;
    ss << "box dump: " << ToString() << endl;
    ss << "\tconnection error due to increasing port connection count";
    throw std::logic_error(ss.str());
  }

}

void Box::ConnectNodeToChannel(const string& node_name, const string& channel_name, const vector<string>& roles) {
  Node& node = this->FindNode(std::move(node_name)).value();
  //Channel& channel = this->FindChannel(std::move(channel_name)).value();

  try {
    node.Connect(channel_name, roles);
  } catch(const std::logic_error& e) {
    std::stringstream ss;
    ss << e.what() << endl;
    ss << "box dump: " << ToString() << endl;
    ss << "\tinternally connecting error";
    throw std::logic_error(ss.str());
  }
}

void Box::TriConnect(vector<string> node_roles, string node_name, string channel_name, string port_name) {
  this->ConnectNodeToChannel(node_name, channel_name, node_roles);
  this->CreatePort(channel_name, port_name);
}

void Box::TriConnect(string node_name, string channel_name, string port_name) {
  this->TriConnect({}, node_name, channel_name, port_name);
}

string Box::ToString(int level) const {
  std::stringstream ss;

  ss << "Box`";
  ss << name_ << ":" << type_;
  ss << "`";

  if (level >= 1) {
    ss << "{ ";
    ss << "nodes: [ ";
    for (const auto& n: nodes) {
      ss << n.ToString(level-1) << " ";
    }
    ss << "] ";

    ss << "channels: [ ";
    for (const auto& c: channels) {
      ss << c.ToString(level-1) << " ";
    }
    ss << "] ";

    ss << "ports: [ ";
    for (const auto& p: ports) {
      ss << p.ToString(level-1) << " ";
    }
    ss << "] ";

    ss << "}";
  }

  return ss.str();
}

}
