#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <functional>
#include <algorithm>
#include <utility>
#include <map>
#include <typeinfo>
#include <any>
#include <variant>
#include <boost/core/demangle.hpp>
#include "ExceptionDecolater.hpp"

using std::cout;
using std::endl;
using std::optional;
using std::string;
using std::vector; // should not use reference to a element of vector.
using std::list; // allow to use reference of a element.
using std::pair;
using std::map;
using std::type_info;
using std::any;

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#define RANGE(v) v.begin(), v.end()
#define UNIQUE(v) {\
  std::sort(RANGE(v));\
  auto last = std::unique(RANGE(v));\
  v.erase(last, v.end());\
}


// Box型が順序付けされていれば BoxType
// 順序付けされていなければ ParentChild
enum class OrderingType {
  BoxType,
  ParentChild
};

map<string,int> map_boxtype_num;
void boxtype_append(string str) {
  map_boxtype_num[str] = map_boxtype_num.size();
}
void init() {
  // the smaller value, the higher priority.
  // descending order.
  //boxtype_append("Internet");
  //boxtype_append("Subnet");
  //boxtype_append("Router");
  boxtype_append("Switch");
  //boxtype_append("Gateway");
  //boxtype_append("Hub");
  boxtype_append("Terminal");
}

optional<int> boxtype_to_num(string str) {
  if (map_boxtype_num.find(str) == map_boxtype_num.end()) return {};
  return map_boxtype_num.at(str);
}

enum class ParentChild {
  Parent, Child
};
enum class BoxOrdering {
  EQ, LT, GT
};
typedef BoxOrdering BoxEquality;

struct BoxPriority {
  string box_type;
  ParentChild parent_child;
};

optional<BoxEquality> CompBoxType(string lhs, string rhs) {
  auto lret = boxtype_to_num(lhs);
  auto rret = boxtype_to_num(rhs);
  if (not (lret and rret)) {
    return {};
    //throw std::logic_error("Exception: could not compare box type, due to no ordering box type!");
  }
  auto lval = lret.value();
  auto rval = rret.value();
  if (lval == rval) return BoxOrdering::EQ;
  else if (lval < rval) return BoxOrdering::LT;
  else if (lval > rval) return BoxOrdering::GT;

  return {};
}

optional<BoxEquality> CompBoxParentChild(ParentChild lhs, ParentChild rhs) {
  if (lhs == rhs) return BoxOrdering::EQ;
  else if (lhs == ParentChild::Parent) return BoxOrdering::LT;
  else if (rhs == ParentChild::Parent) return BoxOrdering::GT;
  return {};
}

BoxEquality CompBoxPriority(const BoxPriority& lhs, const BoxPriority& rhs) {
  if (auto result = CompBoxType(lhs.box_type, rhs.box_type)) {
    return result.value();
  }
  if (auto result = CompBoxParentChild(lhs.parent_child, rhs.parent_child)) {
    return result.value();
  }

  throw std::logic_error("Exception: could not compare box prent-child relation!");
}

bool operator<(const BoxPriority& lhs, const BoxPriority& rhs) {
  if (CompBoxPriority(lhs, rhs) == BoxOrdering::LT) {
    return true;
  }
  else if (CompBoxPriority(lhs, rhs) == BoxOrdering::GT) {
    return false;
  }
  return false;
}
bool operator> (const BoxPriority& lhs, const BoxPriority& rhs){ return rhs < lhs; }
bool operator<=(const BoxPriority& lhs, const BoxPriority& rhs){ return !(lhs > rhs); }
bool operator>=(const BoxPriority& lhs, const BoxPriority& rhs){ return !(lhs < rhs); }
bool operator==(const BoxPriority& lhs, const BoxPriority& rhs) {
  if (CompBoxPriority(lhs, rhs) == BoxOrdering::EQ) {
    //if (lhs.ordering_type == OrderingType::BoxType) {
    //  std::cerr << "warning: no ordering box type!" << endl;
    //}
    //else if (lhs.ordering_type == OrderingType::ParentChild) {
    //  std::cerr << "warning: no parent-child relation!" << endl;
    //}
    return true;
  }
  return false;
}
bool operator!=(const BoxPriority& lhs, const BoxPriority& rhs){ return !(lhs == rhs); }


//////////////////////////////////////////////////

struct Netif {
  string connect;
  vector<string> as;
};

struct Point {
  int x;
  int y;
  optional<int> z;

  Point operator +(const Point& rhs) const {
    Point ret{
      .x = this->x + rhs.x,
      .y = this->y + rhs.y
    };
    if (this->z && rhs.z) {
      ret.z = this->z.value() + rhs.z.value();
    }
    return ret;
  }
};

struct Node {
  string name;
  string type;
  // connection for internal node of box
  vector<string> connect_to; // connecting a channel
  vector<string> roles; // connecting as a role
  //bool is_connected;
  Point point;
  

  void Connect(string channel_name, vector<string> roles) {
    //if (is_connected) {
    //  std::stringstream ss;
    //  ss << "exception: the node already has connected to a channel." << endl;
    //  ss << "\tnode dump: " << ToString(1) << endl;
    //  throw std::logic_error(ss.str());
    //}

    this->connect_to.push_back(channel_name);
    this->roles = roles;
    //is_connected = true;
  }

  void Connect(string channel_name) {
    return Connect(channel_name, {});
  }

  // list of connections, from node to channel
  vector<Netif> GenConnList() const {
    vector<Netif> conn_list;
    for (const auto& conn : this->connect_to) {
      conn_list.push_back({conn, this->roles});
    }

    return conn_list;
  }

  string ToString(int level=0) const {
    std::stringstream ss;
    ss << "Node`" << name << ":" << type << "`";
    if (level >= 1) {
      ss << "{to:[" << std::accumulate(RANGE(connect_to), string{}, [](string a, string b){if(a=="") return b;else return a+b;});
      ss << "],as:" << std::accumulate(RANGE(roles), string{}, [](string a, string b){if(a=="") return b;else return a+b;});
      //ss << "," << is_connected;
      ss << "}";
    }
    return ss.str();
  }
};

struct Channel {
  string name;
  string type;
  optional<string> port; // which port use for this channel
  json config;

  Channel& SetConfig(json val) {
    config = val;
    return *this;
  }

  string ToString(int level=0) const {
    std::stringstream ss;
    ss << "Channel`" << name << ":" << type << "`";
    if (level >= 1) {
      ss << "{port:" << port.value_or("");
      ss << ",conf:" << config;
      ss << "}";
    }
    return ss.str();
  }
};

struct MergedChannel {
  // channels is consist of unique channel name that is given by mangling
  vector<string> channels;
  Channel value;

  string ToString(int level=0) const {
    std::stringstream ss;
    ss << "MChannel{" << value.ToString() << "}";
    return ss.str();
  }
};

struct Port2 {
  string name;
  string from_channel; // name reference
  string to_box;
  string to_box_port;

  int relation; // 0: none, 1:parent-child
  int is_parent;
  //int priority;

  int cur_conn = 0;
  int max_conn = 1;

  // NOTE merged channel temporary put in Port2
  optional<MergedChannel> mchannel;

  Port2& SetMaxConnection(int val) {
    max_conn = val;
    return *this;
  }

  void IncCurConn() {
    if ( cur_conn > max_conn-1 ) {
      std::stringstream ss;
      ss << "exception: `cur_conn = " << cur_conn
         << "` could not increased due to over its maximum `max_conn = " << max_conn << "`." << endl;
      ss << "\tport dump: " << ToString() << endl;
      throw std::logic_error(ss.str());
    }
    cur_conn++;
  }

  string ToString(int level=0) const {
    std::stringstream ss;
    ss << "Port2`" << name << "`{";
    ss << "ch:" << from_channel << ",";
    ss << "bx:" << to_box << ",";
    ss << "ch:" << to_box_port << ",";
    ss << relation << ",";
    ss << is_parent << ",";
    ss << "conn:"<< cur_conn << "/" << max_conn;
    if (mchannel) {
      ss << ",";
      ss << "mch:" << mchannel.value().ToString(level-1);
    }
    ss << "}";

    return ss.str();
  }
};

// box's port connection
struct Port {
  // read only
  string from_channel_type;
  string to_box;
  string to_box_channel_type;

  int relation;  // 0:none, 1:parent-child
  int is_parent; // 1: parent, 0:child
  //int priority;

  // チャネルの融合
  //
  // チャネルの融合はチャネルの型で判断する.
  // 融合されたチャネルは, 複数のBoxで共有される予定.
  // 競合状態に注意する必要がある.
  string GetMergeChannelType() const {
    // TODO チャネルの融合について
    if (relation == 0) {
      return from_channel_type;
    }
    
    if (is_parent == 1) {
      return from_channel_type;
    } else {
      return to_box_channel_type;
    }

    std::stringstream ss;
    ss << "port dump: " << ToString() << endl;
    ss << "\tinvalid port relation";
    throw std::logic_error(ss.str());
  }

  Channel GetMergeChannel() const {
    std::stringstream ss;
    ss << "_M"; // _M prefixed to a merged channel, as signature
    ss << "_C" << from_channel_type; // would not mangling to channel type by name mangling
    ss << to_box;
    ss << to_box_channel_type;
    return {ss.str(), GetMergeChannelType()};
  }

  bool IsParent() const {
    return (is_parent ? true : false);
  }

  string ToString(int level=0) const {
    std::stringstream ss;
    ss << "Port{";
    ss << from_channel_type << ",";
    ss << to_box << ",";
    ss << to_box_channel_type << ",";
    ss << relation << ",";
    ss << is_parent;
    ss << "}";

    return ss.str();
  }
};

// associated event
// premitive event
// - Time
// - Sig
struct Time {
  int value;
  Time(int t): value(t) {}
  Time& operator+=(const Time& rhs) {
    *this = *this + rhs;
    return *this;
  }
  Time operator+(const Time& rhs) const {
    return Time { value + rhs.value };
  }
  bool operator<(const Time& rhs) const {
    return value < rhs.value;
  }
  bool operator> (const Time& rhs) const { return rhs < *this; }
  bool operator<=(const Time& rhs) const { return !(*this > rhs); }
  bool operator>=(const Time& rhs) const { return !(*this < rhs); }
  bool operator==(const Time& rhs) const {
    return value == rhs.value;
  }
  bool operator!=(const Time& rhs) const { return !(*this == rhs); }
  string ToString() const {
    std::stringstream ss;
    ss << "Time{" << value << "}";
    return ss.str();
  }
};

struct Sig {
  string value;
  bool operator==(const Sig& rhs) const {
    return value == rhs.value;
  }
  bool operator!=(const Sig& rhs) const { return !(*this == rhs); }
  string ToString() const {
    std::stringstream ss;
    ss << "Sig{\"" << value << "\"}";
    return ss.str();
  }
  bool operator<(const Sig& rhs) const {
    return value < rhs.value;
  }
  bool operator> (const Sig& rhs) const { return rhs < *this; }
  bool operator<=(const Sig& rhs) const { return !(*this > rhs); }
  bool operator>=(const Sig& rhs) const { return !(*this < rhs); }
};

template<typename T>
struct Range {
  T start;
  T stop;
  T interval;
  Range(T start, T stop, T interval): start(start), stop(stop), interval(interval) {
    if (not (start < stop)) {
      throw std::runtime_error("range construction error: could not be `start < stop`");
    }
    if (interval <= 0) {
      throw std::runtime_error("range construction error: could not specified interval to less than equal 0.");
    }
  }
  Range(T start, T stop): Range(start, stop, 1) {}
};

// box
class Box {
private:
  // [TODO] 名前をポインタとして使ってしまっているので, 別途ポインタを用意する?
  string name_;
  string type_;
  Point point_;

public:
  Box(string name, string type): name_(name), type_(type) {
    // initialization
    point_ = {};
  }

  // [TODO]
  // 現実装では, 名前の変更は接続処理の前でなければならない.
  // 名前はポインタのように扱われるため, 接続後に名前を変更した場合, 動作は未定義.
  //
  // 接続後の名前の変更を禁止にするアイディアはどうだろうか.
  Box& SetName(string name) {
    this->name_ = name;
    return *this;
  }

  Box& SetType(string type) {
    this->type_ = type;
    return *this;
  }

  Box& SetPoint(Point pt) {
    this->point_ = pt;
    return *this;
  }

  string GetName() const {
    return this->name_;
  }

  string GetType() const {
    return this->type_;
  }

  Point GetPoint() const {
    return this->point_;
  }

  Box Fork(string name) const {
    Box ret = *this;
    ret.SetName(name);
    return ret;
  }

  Box Fork(string name, string type) const {
    Box ret = *this;
    ret.SetName(name);
    ret.SetType(type);
    return ret;
  }

  // PrimitiveAction (ActionType, AppType)
  struct PrimitiveAction {
    string type; // ActionType (NSOM-AppType)
    json param;
    PrimitiveAction(string type, json param)
    : type(type), param(param)
    {
      if ( not(param.is_null() or param.is_object()) ){
        std::stringstream ss;
        ss << "`params` of action must be json that ether null or object.";
        throw std::logic_error(ss.str());
      }
    }
    string ToString() const {
      std::stringstream ss;
      ss << "PreAct{"
        << "type:" << type
        << ",param:" << param
        << "}";
      return ss.str();
    }
    bool operator==(const PrimitiveAction& rhs) const {
      return (this->type == rhs.type and this->param == rhs.param);
    }
    bool operator!=(const PrimitiveAction& rhs) const { return !(*this==rhs); }
    bool operator<(const PrimitiveAction& rhs) const {
      if (this->type < rhs.type) return true;
      if (this->param < rhs.param) return true;
      if (this->type > rhs.type) return false;
      if (this->param > rhs.param) return false;
      return false;
    }
    bool operator> (const PrimitiveAction& rhs) const { return rhs < *this; }
    bool operator<=(const PrimitiveAction& rhs) const { return !(*this > rhs); }
    bool operator>=(const PrimitiveAction& rhs) const { return !(*this < rhs); }
  };

  struct Task; //prototype
  struct Schedule {
    // cannot use vector dut to possible invalid reference
    //https://ja.cppreference.com/w/cpp/container/list
    list<Task> tbl;
    list<Task> sigtbl;
    bool operator==(const Schedule& rhs) const {
      return (this->tbl == rhs.tbl and this->sigtbl == rhs.sigtbl);
    }
    bool operator!=(const Schedule& rhs) const { return !(*this==rhs); }
    // no overwrite sigtbl when this has already sigtbl's signal.
    Schedule ConcatSigtable(const Schedule& rhs) const {
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
    string ToString(int l=0) const {
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
  };

  Schedule schedule_;

  using ActionSpecifier=std::variant<PrimitiveAction,Sig,Schedule>;
  static string ActionToString(const ActionSpecifier& act, int l=0) {
    std::stringstream ss;
    ss << "Action{";
    if (const auto& val = std::get_if<PrimitiveAction>(&act)) {
      ss << val->ToString();
    } else if (const auto& val = std::get_if<Sig>(&act)) {
      ss << val->ToString();
    } else if (const auto& val = std::get_if<Schedule>(&act)) {
      ss << endl;
      ss << val->ToString(l);
    } else {
      throw std::logic_error("could not action to string!");
    }
    ss << "}";
    return ss.str();
  }
  // BEGIN-Schedule, receive and action
  // (premitive) Event [ Time(int) | Sig(int) ]
  class Event {
  public:
    using Type = std::variant<Time,Sig>;
  private:
    Type value_;
  public:
    Event(Time value): value_(value) {}
    Event(Sig value): value_(value) {}
    const Type& value() const noexcept { return value_; }
    Event operator+(const Event& rhs) const {
      if (auto&& lhs_val = std::get_if<Time>(&this->value_))
      if (auto&& rhs_val = std::get_if<Time>(&rhs.value_)) {
        return {*lhs_val + *rhs_val};
      }
      throw std::logic_error("exception: operator `+` use only to Time");
    }
    Event operator+=(const Event& rhs) {
      *this = *this + rhs;
      return *this;
    }
    bool operator==(const Event& rhs) const {
      return this->value_==rhs.value_;
    }
    bool operator!=(const Event& rhs) const { return !(*this==rhs); }
    string ToString() const {
      if (auto&& time = std::get_if<Time>(&this->value_)) {
        return time->ToString();
      }
      if (auto&& sig = std::get_if<Sig>(&this->value_)) {
        return sig->ToString();
      }
      throw std::logic_error("exception: no support ToString convertion!");
    }
    bool operator<(const Event& rhs) const {
      return this->value_ < rhs.value_;
    }
    bool operator> (const Event& rhs) const { return rhs < *this; }
    bool operator<=(const Event& rhs) const { return !(*this > rhs); }
    bool operator>=(const Event& rhs) const { return !(*this < rhs); }
  };
  // Event specifer is ether Time, Range<Time>
  class EventSpecifer {
    vector<Event> v_; // OR
  public:
    EventSpecifer() {
      // immediately event
      v_.push_back(Event{Time{0}});
    }
    EventSpecifer(Event evt) {
      v_.push_back(evt);
    }
    EventSpecifer(Time time) {
      v_.push_back(Event{time});
    }
    EventSpecifer(int time) {
      v_.push_back(Event{Time{time}});
    }
    EventSpecifer(Sig id) {
      v_.push_back(Event{id});
    }
    template<typename T>
    EventSpecifer(Range<T> range) {
      if (not (range.start <= range.stop)) throw std::logic_error(
          "Range of event is invalid value "
          "that it is not `start` less than equal `stop`."
          );
      for (T cur = range.start; cur < range.stop; cur += range.interval) {
        this->v_.push_back(Event{cur});
      }
    }
    vector<Event> value() const {
      return v_;
    }
  };

  struct ScheduleControllBlock;
  struct ScheduleRefBox {
    Schedule& value;

    static ScheduleControllBlock
    CreateSCB(ScheduleRefBox srb, optional<Event> evt_last)
    {
      return {srb, evt_last, {}};
    }

    ScheduleControllBlock
    SCB(optional<Event> evt_last)
    {
      return CreateSCB(*this, evt_last);
    }

    optional<ScheduleControllBlock>
    AddTask(Event evt, ActionSpecifier action)
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
        return CreateSCB(ScheduleRefBox{*p_sdl}, p_last->evt); // return SCB
      }

      return {};
    }
    // [Event] -> [Action] -> [SCB]
    vector<ScheduleControllBlock>
    AddTask(EventSpecifer es, ActionSpecifier action)
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
              CreateSCB(ScheduleRefBox{*p_sdl}, p_last->evt) // add SCB
              );
        }
      }

      return scbs;
    }

    Schedule GetSchedule() const {
      return value;
    }
  };
  struct ScheduleControllBlock {
    ScheduleRefBox srb; // reference to the instance of schedule table
    optional<Event> evt_last; // last pushed event
    vector<Event> evts; // accumulate OR events

    ScheduleControllBlock At(EventSpecifer es) {
      vector<Event> v = es.value();
      evt_last = v[v.size()-1];
      this->evts.insert(this->evts.end(), RANGE(v));
      return *this;
    }
    // should be call after Do
    ScheduleControllBlock Aft(EventSpecifer es) {
      if (not evt_last) {
        throw std::runtime_error("Aft failed, dut to no exist befor event");
      }
      // nest
      // .At(0).Aft(t) -> .At(0).Sdl{.At(1).Sdl{return here_scb}}
      optional<ScheduleControllBlock>
        nest = this->srb.AddTask(evt_last.value(), Schedule{});
      ScheduleControllBlock nest_scb = nest.value();
      vector<ScheduleControllBlock>
        nest2 = nest_scb.srb.AddTask(es, Schedule{});// TODO AddTask with EventSpecifer
      ScheduleControllBlock nest_scb2 = nest2[nest2.size()-1]; //return the last of element
      return nest_scb2;
    }
    ScheduleControllBlock Do(string act_type, json param) {
      if (this->evts.size() == 0) this->At(0);
      for (const auto& e : this->evts) {
        srb.AddTask(e, PrimitiveAction{act_type, param});
      }
      return {this->srb, this->evt_last}; //reset
    }
    ScheduleControllBlock Do(Sig sig) {
      if (this->evts.size() == 0) this->At(0);
      for (const auto& e : this->evts) {
        srb.AddTask(e, sig);
      }
      return {this->srb, this->evt_last}; //reset
    }
    ScheduleControllBlock Do(Schedule sdl) {
      if (this->evts.size() == 0) this->At(0);
      for (const auto& e : this->evts) {
        srb.AddTask(e, sdl);
      }
      return {this->srb, this->evt_last}; //reset
    }
    ScheduleControllBlock Sdl(
        const std::function<void(ScheduleControllBlock&)>& cb)
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
  };

  struct DecolateEventBox {
    Box& box;
    optional<Event> evt_last;
    vector<Event> evts;

    DecolateEventBox At(EventSpecifer es) {
      vector<Event> v = es.value();
      evt_last = v[v.size()-1];
      this->evts.insert(this->evts.end(), RANGE(v));
      return *this;
    }
    DecolateEventBox Do(string act_type, json param) {
      // generate application
      // params: [ HandlerName, HandlerParameter ]
      for (const auto& e : this->evts) {
        box.AddTask(e, PrimitiveAction{act_type, param});
      }
      return {this->box, this->evt_last}; //reset
      //return this->box;
    }
    DecolateEventBox Do(Sig sig) {
      for (const auto& e : this->evts) {
        box.AddTask(e, sig);
      }
      return {this->box, this->evt_last}; //reset
      //return this->box;
    }
    DecolateEventBox Do(Schedule sdl) {
      for (const auto& e : this->evts) {
        //cout << "[DEBUG] action: evt " << e.ToString() << endl;
        box.AddTask(e, sdl);
      }
      return {this->box, this->evt_last}; //reset
      //return this->box;
    }
    DecolateEventBox Schedule(const std::function<void(Box&)>& cb) {
      Box sdl_box("tmp", "tmp");
      cb(sdl_box);
      return Do(sdl_box.GetSchedule());
    }
    //Box& Do(string sig) {
    //  return Do(Sig{sig});
    //}
  };
  // Box::At method is bootstrap of ScheduleControllBlock
  ScheduleControllBlock At(EventSpecifer es) {
    return ScheduleRefBox{this->schedule_}.SCB({}).At(es);
  }

  // Task is move-only and use only via reference.
  struct Task {
    Event evt; // receive event
    ActionSpecifier action; // action

    string ToString(int l=0) const {
      std::stringstream ss;
      ss << "Task{evt="
        << evt.ToString()
        << ",act="
        << ActionToString(action, l+2)
        << "}";
      return ss.str();
    };

    bool operator==(const Task& rhs) const {
      return (
          evt == rhs.evt &&
          action == rhs.action
          );
    }
    bool operator!=(const Task& rhs) const {
      return !(*this == rhs);
    }
  };

  // find tasks that have the event`evt` as a action.
  // T: parent task.aciton type
  // U: type of variant
  template<typename T, typename U>
  static vector<Task> FindTasksActionAs(const vector<Task>& tasks, const U& value) {
    vector<Task> ret;
    if (auto&& evt_act = std::get_if<T>(&value)) {
      for (const auto& task : tasks)
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

  static vector<Event> SearchPrimitiveEvents(const vector<Task>& sdl, const Task& task, vector<Task>& breadcrumb) {
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
  vector<Event> SearchPrimitiveEvents(const vector<Task>& sdl, const Task& task) const {
    vector<Task> task_crumb; // avoid for duplication search
    return SearchPrimitiveEvents(sdl, task, task_crumb);
  }

private:
  // not recommendation for user to use
  void AddTask(Event evt, ActionSpecifier action) {
    if (std::get_if<Sig>(&evt.value())) {
      this->schedule_.sigtbl.push_back(Task{evt, action});
    } else {
      this->schedule_.tbl.push_back(Task{evt, action});
    }
  }
public:
  Schedule GetSchedule() const {
    return this->schedule_;
  }

  string DumpSchedule() const {
    std::stringstream ss;
    ss << schedule_.ToString();
    return ss.str();
  }

private:
  vector<Task> FlattenSchedule(const Schedule& sdl,int callid=0) const {
    vector<Task> ret;
    cout << "[DEBUG!!CALL("<<callid<<")]" << endl;
    cout << sdl.ToString() << endl;
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
      // task is Schedule
      if (auto&& ptr = std::get_if<Schedule>(&task.action)) {
        const auto& nested_sdl = *ptr;
        const auto& temp = nested_sdl.ConcatSigtable(sdl);

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

    cout << "[DEBUG!!EXIT("<<callid<<")]" << endl;
    int idx = 0;
    for (auto&& i : ret) {
      cout << "["<<(idx++)<<"] " << i.ToString() << endl;
    }

    return ret;
  }
public:
  vector<Task> FlattenSchedule() const {
    return FlattenSchedule(this->schedule_);
  }
  void ClearSchedule() {
    this->schedule_ = {};
  }
public:
  // END-Schedule

  // resolve the merged channel from a channel.
  // channel_name is channel's name of a node-to-channel connection.
  // 1. search channel from inner box
  // 2. channel resolve to merged-channel
  // 3. return merged-channl
  optional<string> ResolveConn(string channel_name) const {
    try {
      Channel& ch = this->FindChannel(channel_name).value();
      Port2& port = this->FindPort2(ch.port.value()).value();
      MergedChannel mch = port.mchannel.value();
      return mch.value.name; // merged channel name
    } catch(const std::bad_optional_access& e) {
      return {};
    }

  }

  vector<Netif> GenNodeConnList(const Node& node) const {
    vector<Netif> node_conn_list = node.GenConnList();
    // connection list of node-to-channel resolve to node-to-merged-channel
    for (auto&& netif : node_conn_list) {
      netif.connect = this->ResolveConn(netif.connect).value_or(netif.connect);
    }
    return node_conn_list;
  }

private:
  string PrefixBox(string name) const {
      std::stringstream ss;
      ss << "_B" << name;
      return ss.str();
  }
  string PrefixNode(string name) const {
      std::stringstream ss;
      ss << "_N" << name;
      return ss.str();
  }
  string PrefixChannel(string name) const {
      std::stringstream ss;
      ss << "_C" << name;
      return ss.str();
  }
  string PrefixPort(string name) const {
      std::stringstream ss;
      ss << "_P" << name;
      return ss.str();
  }
public:
  
  // name mangling for internal nodes, channels and ports.
  // type prefix;
  // - Box     : B
  // - Node    : N
  // - Channel : C
  void Mangle() {
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

  Node& CreateNode(string name, string type) {
    nodes.push_back(Node{name, type});
    return FindNode(name).value();
  }

  Node& CopyNode(string src, string dst) {
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

  Channel& CreateChannel(string name, string type) {
    channels.push_back(Channel{name, type});
    return FindChannel(name).value();
  }

  Channel& CopyChannel(string src, string dst) {
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

  optional<std::reference_wrapper<Channel>> FindChannel(string name) const {
    for (Channel& i : channels) {
      if (i.name == name) return i;
    }
    return {};
  }

  optional<std::reference_wrapper<Node>> FindNode(string name) const {
    for (Node& i : nodes) {
      if (i.name == name) return i;
    }
    return {};
  }

  optional<std::reference_wrapper<Port2>> FindPort2(string name) const {
    for (Port2& i : ports) {
      if (i.name == name) return i;
    }
    return {};
  }

  optional<string> CheckInvalidChannel(const Channel& from_channel, const Channel& to_channel) {
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
  Port2& CreatePort(const string& from_this_channel_name, const string& port_name) {
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
  void SetPortMaxConnection(const string& port_name, int num) {
    Port2& port = FindPort2(std::move(port_name)).value();
    port.max_conn = num;
  }

  void ConnectPort(const string& from_this_port_name, Box& to_box, const string& to_box_port_name) {
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

  void ConnectNodeToChannel(const string& node_name, const string& channel_name, const vector<string>& roles = {}) {
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

  void TriConnect(vector<string> node_roles, string node_name, string channel_name, string port_name) {
    this->ConnectNodeToChannel(node_name, channel_name, node_roles);
    this->CreatePort(channel_name, port_name);
  }

  void TriConnect(string node_name, string channel_name, string port_name) {
    this->TriConnect({}, node_name, channel_name, port_name);
  }

  string ToString(int level=0) const {
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

  // NOTE mutable is meaning that allow returning reference from 'const function'
  mutable list<Node> nodes;
  mutable list<Channel> channels;
  mutable list<Port2> ports; // ports is channels which has used by box connecting.
};

class NsomBuilder {
private:
  vector<Box> boxs_;
  string name_;

public:
  NsomBuilder(string name)
  : name_(name)
  {}

  void AddBox(const Box& box) {
    boxs_.push_back(box);
  };

  // build ns-3 code
  string Build() {
    vector<Box> boxs = boxs_;
    for (auto&& b : boxs) {
      // name mangling to avoid NSOM item's name conflict.
      b.Mangle();

      cout << "[DEBUG] " << b.ToString(100) << endl;
      cout << "[DEBUG] " << b.DumpSchedule() << endl;
    }

    // TODO marge channel preprocess
    MergeChannel(boxs);

    json j;
    j["name"]    = name_;
    j["node"]    = GenNodeList(boxs);
    j["channel"] = GenChannelList(boxs);
    j["subnet"]  = GenSubnetList();
    j["apps"]    = GenAppsList(boxs);

    cout << "[builder] build finish!" << endl;
    cout << "[builder] network name: "  << j["name"] << endl;
    cout << "[builder] generated node: "    << j["node"].size() << endl;
    cout << "[builder] generated channel: " << j["channel"].size() << endl;
    cout << "[builder] generated subnet: "  << j["subnet"].size() << endl;
    cout << "[builder] generated apps: "    << j["apps"].size() << endl;

    return j.dump(2);
  }

  optional<std::reference_wrapper<Box>>
  FindBox(string name, optional<std::reference_wrapper<vector<Box>>> target_boxs = {}) {
    vector<Box>& boxs = (target_boxs ? target_boxs.value().get() : boxs_);
    for (size_t i = 0; i < boxs.size(); ++i) {
      if (boxs[i].GetName() == name) return boxs[i];
    }
    return {};
  }

  optional<std::reference_wrapper<Port2>> FindRelevantPort2(Port2 port, vector<Box>& boxs) {
    Box& to_box = FindBox(port.to_box, boxs).value();
    return to_box.FindPort2(port.to_box_port).value();
  }

  BoxPriority ToBoxPriority(const Box& box, const Port2& port) {
    return {
      box.GetType(), (port.is_parent==1 ? ParentChild::Parent : ParentChild::Child)
    };
  }

  MergedChannel GetMergeChannel(vector<Box> boxs, Box parent_box, Port2 parent_port) {
    // parent_box.port.from_channel -> parent_box.channel
    Channel& from_channel = parent_box.FindChannel(parent_port.from_channel).value();
    // parent_box.port.to_box_name -> to_box
    //   -> parent_box.port.to_box_port_name -> to_box.port
    //   -> to_box.channel
    Box& child_box = FindBox(parent_port.to_box, boxs).value();
    Port2& child_box_port = child_box.FindPort2(parent_port.to_box_port).value();
    Channel& to_channel = child_box.FindChannel(child_box_port.from_channel).value();

    std::stringstream ss;
    ss << "_M"; // _M prefixed to a merged channel, as signature
    ss << parent_box.GetName();
    ss << parent_port.name;
    ss << child_box.GetName();
    ss << child_box_port.name;

    MergedChannel ret = {
      .channels = {from_channel.name, to_channel.name},
      .value = {ss.str(), ""}
    };

    // TODO merge
    //cout << "[DEBUG] parent_box " << parent_box.ToString(1) << endl;
    //cout << "[DEBUG] child_box " << child_box.ToString(1) << endl;
    BoxPriority bp_parent = ToBoxPriority(parent_box,parent_port);
    BoxPriority bp_child = ToBoxPriority(child_box,child_box_port);

    if (bp_parent < bp_child) {
      ret.value.type = from_channel.type;
      ret.value.config = from_channel.config;
      return ret;
    } else if (bp_parent > bp_child) {
      ret.value.type = to_channel.type;
      ret.value.config = to_channel.config;
      return ret;
    }

    throw std::logic_error("invalid port relation");
  }

  void MergeChannel(vector<Box>& boxs) {
    for (auto& box : boxs)
    for (auto& port : box.ports) {
      if (port.is_parent) {
        // decide marged channel
        MergedChannel mch = GetMergeChannel(boxs, box, port);
        // set the marged channel to the parent box's port
        port.mchannel = mch;
        Port2& child_port = FindRelevantPort2(port, boxs).value();
        child_port.mchannel = mch;
      }
    }

  }

public:
  json GenNodeList(const vector<Box>& boxs) const {
    json j;
    for (const auto& box : boxs) {
      for (const auto& node : box.nodes) {
        // set a node's name to a key,
        // the node's body create, and set
        j[node.name] = GenNodeBody(box, node);
      }
    }
    return j;
  }

private:
  json GenNodeBody(const Box& box, const Node& node) const {
    json j;

    // set point x, y and optional z
    auto pt = node.point + box.GetPoint();
    j["point"] = {{"x", pt.x},{"y", pt.y}};
    if (pt.z) j["point"]["z"] = pt.z.value();

    // set netifs
    j["netifs"] = GenNetifs(box.GenNodeConnList(node));

    return j;
  }
  json GenNetifs(const vector<Netif>& conn_list) const {
    vector<json> netifs;
    for (const auto& conn : conn_list) {
      netifs.push_back(GenNetif(conn));
    }

    return netifs;
  }
  json GenNetif(const Netif& conn) const {
    json netif;
    
    // set "connect"(distination) and optional "role"
    // to "connect" and "as" respectively.
    // convert a connection node-to-channel to node-to-merged-channel.
    netif["connect"] = conn.connect;
    if (conn.as.size() == 1) netif["as"] = conn.as[0];
    else if (conn.as.size() >= 2) netif["as"] = conn.as;

    return netif;
  }

public:
  json GenChannelList(const vector<Box> boxs) {
    json j;
    for (const auto& box : boxs) {
      for (const auto& port : box.ports) {
        // set a node's name to a key,
        // the node's body create, and set
        if (port.is_parent) {
          //Channel ch = GetMergeChannel(boxs, box, port);
          Channel ch = port.mchannel.value().value;
          j[ch.name] = GenChannelBody(ch);
        }
      }
    }
    for (const auto& box : boxs) {
      for (const auto& ch : box.channels) {
        // set a node's name to a key,
        // the node's body create, and set
        if (not ch.port) j[ch.name] = GenChannelBody(ch);
      }
    }

    return j;
  }

  json GenChannelBody(Channel channel) {
    json j;
    j["type"] = channel.type;
    //cout << "[DEBUG]" << channel.ToString(1) << endl;
    if (channel.config.is_object()) {
      j["config"] = channel.config;
    }

    return j;
  }

public:
  // TODO
  json GenSubnetList() {
    return {};
  }
  json GenAppsList(const vector<Box> boxs) {
    json j;
    for (const auto& box : boxs) {
      int task_i = 0;
      for (const auto& task : box.FlattenSchedule()) {
        cout << "[DEBUG] " << task.ToString() << endl;
        string task_name = box.GetName() + "_T" + std::to_string(task_i);
        j[task_name] = GenAppBody(task);
        ++task_i;
      }
    }

    return j;
  }
  json GenAppBody(const Box::Task& task) {
    json j;
    try {
      const auto& action = std::get<Box::PrimitiveAction>(task.action);
      j["type"] = action.type;
      j["args"] = action.param;
    }
    catch (const std::bad_variant_access&) {
      std::stringstream ss;
      ss << "exception: task.action is not type of PrimitiveAction!\n";
      std::visit([&ss](auto&& arg) {
          using T = std::decay_t<decltype(arg)>;
          ss << "\ttask.action type: " << boost::core::demangle(typeid(T).name());
          }, task.action);
      throw std::logic_error(ss.str());
    }
    if (auto&& v = std::get_if<Time>(&task.evt.value())) {
      const Time& time = *v;
      j["args"]["start"] = time.value;
    } else {
      std::stringstream ss;
      ss << "Could not generate application due to event type `";
      std::visit([&ss](auto&& evt){
          using T = decltype(evt);
          ss << boost::core::demangle(typeid(T).name());
          }, task.evt.value());
      ss << "`." << endl;
      ss << "It event is not implemented currently...";
      throw std::runtime_error(ss.str());
    }
    return j;
  }
};

void bridge_test2() {
  // ISSUE box-type ordering for priority?
  // e.g. BridgeBox > TerminalBox > ParentChild
  // this would be used for channel merge.
  //
  // BoxTypeOrder box_order;
  // order.SetOrder({
  //   "BridgeBox",
  //   "TerminalBox",
  //   "Box"
  // });
  // NodeTypeOrder node_order;
  // order.SetOrder({
  //   "BridgeNode",
  //   "TerminalNode",
  //   "Node"
  // });
  // DefaultTypeOrder default_order;
  // order.SetOrder({
  //   "Bridge",
  //   "Terminal",
  //   "Node"
  // });
  // ??.SetOrder({
  //   { Ordering::BoxType, "SwitchBox" },
  //   { Ordering::BoxType, "TerminalBox" },
  //   { Ordering::ParentChild }
  // });
  // builder.SetOrder(default_order);

  // ordering for box type to use when channel merge
  //boxtype_append("TerminalBox");
  boxtype_append("SwitchBox");
  boxtype_append("TerminalBox");

  /* create switch-box */
  Box b0("bridge", "SwitchBox");
  b0.CreateNode("n0", "Node");
  // channel creation and configuration for bridge
  b0.CreateChannel("c0", "SwicthChannel")
    .SetConfig({ {"Address", { {"Base", "192.168.10.1/24"}, {"Type", "NetworkUnique"} }}});
  b0.TriConnect({"Switch"}, "n0", "c0", "p0");
  
  /* create terminal-box */
  Box b1("b1", "TerminalBox");
  b1.CreateNode("n0", "Node");
  b1.CreateChannel("c0", "TerminalChannel");
  b1.TriConnect("n0", "c0", "p0");

  // set points
  b0.SetPoint({10,10});
  b1.SetPoint({10,0});

  // connect box
  b1.ConnectPort("p0", b0, "p0");

  // NSOM build
  NsomBuilder builder("TestNet");
  builder.AddBox(b0);
  builder.AddBox(b1);
  cout << builder.Build() << endl;
}

void receive_and_action_test() {
  Box b0("b0", "Box");
  b0.CreateNode("n0", "Node");
  b0.CreateChannel("c0", "Csma");
  b0.TriConnect("n0", "c0", "p0");
  Box b1 = b0.Fork("b1");
  b0.ConnectPort("p0", b1, "p0");

  // schedule test
  json ping_param = {
    {"dhost", "${_Bb0_Nn0}" }, // TODO resolve node name
    //         ^ b0 ... as box, ping handler receive it?
    {"dport", 8080 },
    {"time" , 5 },
    {"rate" , "1Mbps" }
  };
  //b0.At(5)
  //  .Do("ping", ping_param)
  //  .At(10)
  //  .Do("ping", ping_param)
  //  ;
  //b0.Sdl(Time{15}, "ping", {{"shost", "${_Bb0_Nn0}"}});

  b0.At(Range<Time>{5, 15, 5}) // expanded to [5,10,15]
    .Do("Ping", ping_param)
    ;
  
  b1.At(1)
    .Do("Sink", {{"port",8080},{"time",30}})
    ;

  // TODO signal (rough draft)
  //b1.At(Sig{"dummy"})
  //  .Do("Sig", {{"param1",123},{"param2","abc"}})
  //  ;

  NsomBuilder builder("TestNet");
  builder.AddBox(b0);
  builder.AddBox(b1);
  cout << builder.Build() << endl;
}

void signal_test() {
  Box Base("sender_base", "Base");
  Base.CreateNode("n0", "Node");
  Base.CreateChannel("c0", "PointToPoint");
  Base.TriConnect("n0", "c0", "p0");

  Box sender = Base.Fork("sender", "Sender");
  Box sinker = Base.Fork("sinker", "Sinker");
  sender.ConnectPort("p0", sinker, "p0");

  //cout << "[TEST(1)]" << endl;
  //sender
  //  .At(Time{1}).Do(Sig{"Sig1"})
  //  .At(Time{1}).Do(Sig{"Sig2"}) // simultaneous do signal
  //  .At(Time{2}).Do(Sig{"Sig1"})
  //  .At(Sig{"Sig1"}).Do(Sig{"Sig3"})
  //  .At(Sig{"Sig2"}).Do("Pre1", {})
  //  .At(Sig{"Sig2"}).Do(Sig{"Sig4"})
  //  .At(Sig{"Sig3"}).Do("Pre2", {})
  //  .At(Sig{"Sig3"}).Do(Sig{"Sig2"})
  //  ;
  //for (const auto& i : sender.FlattenSchedule()) {
  //  cout << "[test] " << i.ToString() << endl;
  //}
  //sender.ClearSchedule();

  //cout << "[TEST(2)]" << endl;
  //sender
  //  .At(Time{1}).Do(Sig{"Sig1"})
  //  .At(Sig{"Sig1"}).Do("Pre1",{})
  //  .At(Sig{"Sig1"}).Do("Pre2",{})
  //  ;
  //for (const auto& i : sender.FlattenSchedule()) {
  //  cout << "[test] " << i.ToString() << endl;
  //}
  //sender.ClearSchedule();

  cout << "[TEST(3)]" << endl;

  // Aft(time).Do(action)
  // =
  // Aft: (box:Box, base:Time, t:Time, act:Action) -> Box
  // box.
  //   .At(base).Do(Sig{"unique_sig"})
  //   .At(Sig{"unique_sig"}).Schedule([](auto&&a){a
  //     .At(t).Do(act)
  //     return a;
  //   });
  ////Box Aft(Box box, Time base, Time t, Action act)
  ////{
  ////  Box ret;
  ////  box
  ////    .At(base)
  ////    .Sdl([&ret](auto&&a){a
  ////      .At(t).Do(act)
  ////      ret = a;
  ////    });
  ////  return a;
  ////}

#define SB Sdl([](auto&&a){a
#define SE })
  //sender
  //  .At(Time{0}).Do("Ping",{})
  //  .At(Time{0}).SB
  //    .At(Time{1}).Do("Ping",{})
  //    ;
  //  SE
  //  ;
  //sender
  //  .At(0).Do("Prepare",{})
  //  .Aft(10).Do(Sig{"Ready"})

  //  .At(Sig{"Ready"}).Aft(1).Do("Send",{})

  //  .At().Do("Ping",{})
  //  .At().Do("FlowPattern",{})
  //  .At().Do("Sink",{})
  //  .At().Do("NicCtl",{})
  //  .At().Do("Move",{})
  //  ;

  sender
    //.At(Time{0}).Sdl([](auto&&a){a
    //.At(Sig{"world"}).Do("Hoge",{})
    .At(10).Sdl([](auto&&a){a
      .At(1).Do(Sig{"hello"}) // 11
      .At(2).Do(Sig{"world"}) // 12
      ;
    })
    .At(Sig{"hello"}).Do("Honma",{}) // 11*
    .At(Sig{"world"}).Do("Himawari",{}) // 12*
      .Aft(1) // 13
      .At(1).Do("🌻",{}) // 14*
      .At(0).Do("🌻",{}) // 13*
    ;
  cout << sender.DumpSchedule() << endl;
  for (const auto& i : sender.FlattenSchedule()) {
    cout << "[test] " << i.ToString() << endl;
  }
  sender.ClearSchedule();

  /// TEST
  sender
    .At(Sig{"Sig1"})
    .Do("Pre1", {})
    .At(Sig{"Sig2"})
    .Do("Pre1", {})
    ;
  sender
    .At(Time{1}).Do(Sig{"Sig1"})
    .At(Time{2}).Do(Sig{"Sig2"})
    ;
  NsomBuilder builder("TestNet");
  builder.AddBox(sender);
  builder.AddBox(sinker);
  //cout << builder.Build() << endl;
}

void nic_switch_test() {
  // create base box
  Box RouteSwitch("route_switch_base", "RouteSwitch");
  Box Router("router_base", "Router");
  Box Sinker("sinker_base", "Sinker");

  // TODO implement ANY-channel
  // create node and channel over the box.
  RouteSwitch.CreateNode("n0", "Node");
  RouteSwitch.CreateChannel("c0", "ANY");
  RouteSwitch.CreateChannel("c1", "ANY");
  RouteSwitch.TriConnect("n0", "c0", "p0");
  RouteSwitch.TriConnect("n0", "c1", "p1");

  Router.CreateNode("n0", "Node");
  Router.CreateChannel("c0", "Csma");
  Router.CreateChannel("c1", "Csma");
  Router.TriConnect("n0", "c0", "p0");
  Router.TriConnect("n0", "c1", "p1");

  Sinker.CreateNode("n0", "Node");
  Sinker.CreateChannel("c0", "ANY");
  Sinker.CreateChannel("c1", "ANY");
  Sinker.TriConnect("n0", "c0", "p0");
  Sinker.TriConnect("n0", "c1", "p1");
  Sinker
    .At(Sig{"Start"})
    .Do("Sink", {{"port",8080},{"time",30}})
    ;

  // preparate application(schedule)
  RouteSwitch
    .At(Sig{"SwitchPort0"})
    .Sdl([](auto&&a){a
      .At(Time{0}).Do("NicCtl", {{"idx", "1"},{"enable", "0"}})
      .At(Time{0}).Do("NicCtl", {{"idx", "0"},{"enable", "1"}})
      //.At(Time{0}).Do("PRE_0", {})
      ;
    })
    //.At(Sig{"SwitchPort1"}).Do("PRE_1", {})
    .At(Sig{"SwitchPort1"})
    .Do("NicCtl", {{"idx", "0"},{"enable", "0"}})
    .At(Sig{"SwitchPort1"})
    .Do("NicCtl", {{"idx", "1"},{"enable", "1"}})
    ;

  // topology
  //
  //  RS              Router      Sinker
  // ┌─────┐         ┌─────┐     ┌─────┐
  // │   p0│─────────│p0 p1│─────│p0   │
  // │   p1│────┐    └─────┘    ┌│p1   │
  // └─────┘    │    ┌─────┐    │└─────┘
  //            └────│p0 p1│────┘
  //                 └─────┘
  //
  
  Box rs = RouteSwitch.Fork("rs");
  Box r0 = Router.Fork("r0");
  Box r1 = Router.Fork("r1");
  Box s0 = Sinker.Fork("s0");

  rs.ConnectPort("p0", r0, "p0");
  rs.ConnectPort("p1", r1, "p0");

  s0.ConnectPort("p0", r0, "p1");
  s0.ConnectPort("p1", r1, "p1");

  // schedule
  // global signal: SimStart
  Box global("_global", "GlobalSig"); // exclusive box for signal
  global
    .At(Time{5})
    .Do(Sig{"SimStart"})
    ;
  // sinker
  s0
    .At(Time{5})
    .Do(Sig{"SimStart"})
    .At(Sig{"SimStart"})
    .Do(Sig{"Start"})
    ;
  // route switch
  rs
    .At(Sig{"SwitchPort0"})
    .Do("ADD_PRE_0", {})
    .At(Sig{"Start"})
    .Sdl([](auto&&rs){rs
      .At(Sig{"SwitchPort1"})
      .Do("OVERRIDE_PRE_1", {})
      .At(Range<Time>{10,30,10}) // expanded to [10, 20]
      .Sdl([](auto&&rs){rs
        .At(Time{0})
        .Do(Sig{"SwitchPort0"})
        .At(Time{5})
        .Do(Sig{"SwitchPort1"})
        ;
      })
      .At(Sig{"Timeout"})
      .Do(Sig{"Stop"})
      ;
    })
    ;
  rs
    .At(Time{5})
    .Do(Sig{"SimStart"})
    .At(Sig{"SimStart"})
    .Do(Sig{"Start"})
    ;

  // DEBUG
  //cout << "[DEBUG Schedule]" << endl;
  //cout << rs.DumpSchedule() << endl;

  // build
  NsomBuilder builder("TestNet");
  builder.AddBox(global);
  builder.AddBox(rs);
  builder.AddBox(r0);
  builder.AddBox(r1);
  builder.AddBox(s0);
  cout << builder.Build() << endl;
}


void test() {
  // logic_error: library responsibility
  // runtime_error: user responsibility
  try{
  //receive_and_action_test();
  signal_test();
  //nic_switch_test();
  }catch(const std::exception& e) {
    std::cerr
      //<< e.what()
      << edc::HonhimaDecolate(e).what()
      //<< edc::HondaDecolate(e).what()
      << std::flush;
  }
  cout << "バグなし！生きてるだけで勝ち🌻" << endl;
}

int main() {
  test();
  return 0;
}
