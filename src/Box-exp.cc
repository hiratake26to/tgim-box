#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <utility>
#include <map>

using std::cout;
using std::endl;
using std::optional;
using std::string;
using std::vector;
using std::pair;
using std::map;

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#define RANGE(v) v.begin(), v.end()


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
    vector<Netif> list;
    for (const auto& conn : this->connect_to) {
      list.push_back({conn, this->roles});
    }

    return list;
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

class Box {
private:
  // [TODO] 名前をポインタとして使ってしまっているので, 別途ポインタを用意したい.
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

  // TODO
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
    for (size_t i = 0; i < channels.size(); ++i) {
      if (channels[i].name == name) return channels[i];
    }
    return {};
  }

  optional<std::reference_wrapper<Node>> FindNode(string name) const {
    for (size_t i = 0; i < nodes.size(); ++i) {
      if (nodes[i].name == name) return nodes[i];
    }
    return {};
  }

  optional<std::reference_wrapper<Port2>> FindPort2(string name) const {
    for (size_t i = 0; i < ports.size(); ++i) {
      if (ports[i].name == name) return ports[i];
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
  mutable vector<Node> nodes;
  mutable vector<Channel> channels;
  mutable vector<Port2> ports; // ports is channels which has used by box connecting.
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
    // name mangling to avoid NSOM item's name conflict.
    vector<Box> boxs = boxs_;
    for (auto&& b : boxs) {
      b.Mangle();
      cout << b.ToString(100) << endl;
    }

    // TODO marge channel preprocess
    MergeChannel(boxs);

    json j;
    j["name"]    = name_;
    j["node"]    = GenNodeList(boxs);
    j["channel"] = GenChannelList(boxs);
    j["subnet"]  = GenSubnetList();
    j["apps"]    = GenAppsList();

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

  // TODO check
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
    cout << "[DEBUG] parent_box " << parent_box.ToString(1) << endl;
    cout << "[DEBUG] child_box " << child_box.ToString(1) << endl;
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

#if 0
    // FIXME to_channel, from_channel の返却はあっているか?
    // チャネルの融合
    // 対等な接続なのでどれを取ってもよい
    if (to_box_port.relation == 0) {
      ret.value.type = from_channel.type;
      //ret.value.config = to_channel.config + from_channel.config; // TODO configuration merge
      ret.value.config = "\"configration merge dummy\"";
      return ret;
    }
    
    // 親子関係があるので親から取る
    if (to_box_port.is_parent == 1) {
      ret.value.type = to_channel.type;
      ret.value.config = to_channel.config;
      return ret;
    } else {
      ret.value.type = from_channel.type;
      ret.value.config = from_channel.config;
      return ret;
    }
#endif

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
    j["netifs"] = GenNetifs(box.GenNodeConnList(node)); // TODO check here

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
        // TODO check for convertion result
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
        // TODO invastigate channel generation
        if (not ch.port) j[ch.name] = GenChannelBody(ch);
      }
    }

    return j;
  }

  json GenChannelBody(Channel channel) {
    json j;
    j["type"] = channel.type;
    cout << "[DEBUG]" << channel.ToString(1) << endl;
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
  json GenAppsList() {
    return {};
  }

};

void merge_channel_test() {
  // create box
  Box b0("b0", "Box");
  b0.CreateNode("n", "Node");
  b0.CreateChannel("c", "Channel");
  // internally connect
  b0.ConnectNodeToChannel("n", "c");

  // create port
  b0.CreatePort("c", "p");
  // TODO to be studied
  //b0.CreatePort("c", "c");
  //b0.PortForward("c");
  //b0.Portalize("c"); // otherwise
  //b0.CreatePort("c"); // create and portalize to channel "c"
  
  // fork
  Box b1 = b0.Fork("b1");

  b0.ConnectPort("p", b1, "p");

  // builder instanciate
  NsomBuilder builder("test-net");
  builder.AddBox(b0);
  builder.AddBox(b1);
  cout << builder.Build() << endl;
}

void multi_inner_node_test() {
  /* multi inner node connection
   *
   * b0
   * +----------------+
   * | n0 -> c0 -> [p0|--+
   * | n1 -> c1 -> [p1|--|--+
   * +----------------+  |  |
   *                     |  |
   * b1                  |  |
   * +----------------+  |  |
   * | n0 -> c0 -> [p0|--+  |
   * | n1 -> c1 -> [p1|-----+
   * +----------------+
   */
  // create box
  Box b0("b0", "Box");
  b0.CreateNode("n0", "Node");
  b0.CreateNode("n1", "Node");
  b0.CreateChannel("c0", "Channel");
  b0.CreateChannel("c1", "Channel");
  // internally connect
  b0.ConnectNodeToChannel("n0", "c0");
  b0.ConnectNodeToChannel("n1", "c1");

  // create port
  b0.CreatePort("c0", "p0");
  b0.CreatePort("c1", "p1");
  //b0.SetPortMaxConnection("p", 2);
  // TODO to be studied
  //b0.CreatePort("c", "c");
  //b0.PortForward("c");
  //b0.Portalize("c"); // otherwise
  //b0.CreatePort("c"); // create and portalize to channel "c"
  
  // fork
  Box b1 = b0.Fork("b1");

  b0.ConnectPort("p0", b1, "p0");
  b0.ConnectPort("p1", b1, "p1");

  // builder instanciate
  NsomBuilder builder("test-net");
  builder.AddBox(b0);
  builder.AddBox(b1);
  cout << builder.Build() << endl;
}

void box_self_port_connect_test() {
  /* Multiple inner node connection
   *
   * b0
   * +----------------+
   * | n0 -> c0 -> [p0>--+
   * | n1 -> c1 -> [p1>--|--+
   * +----------------+  |  |
   *                     |..|..merged channel decide by b0.c0
   * b1                  |  |....merged channel decide by b0.c1
   * +----------------+  |  |
   * | n0 -> c0 -> [p0<--+  |
   * | n1 -> c1 -> [p1<-----+
   * |  |           ︙|
   * |  +--> c2 -> [p2|  what is connection of p1 and p2
   * +----------------+
   *
   * NOTE
   * It would be necessary, first, that solve box internal connection?
   * That would be equivalent following?
   * ...
   * | n1 -> c1 -> [p1|       | n1 ->|c1|-> [p1|
   * |  |           ︙|       |      |  |    ︙|
   * |  +--> c2 -> [p2|   ,   |      |c2|-> [p2|
   * ...
   * even included in the same box, both c1 and c2 should be merged.
   */
  // create box
  Box b0("b0", "Box");
  b0.CreateNode("n0", "Node");
  b0.CreateNode("n1", "Node");
  b0.CreateChannel("c0", "Channel");
  b0.CreateChannel("c1", "Channel");
  // internally connect
  b0.ConnectNodeToChannel("n0", "c0");
  b0.ConnectNodeToChannel("n1", "c1");

  // create port
  b0.CreatePort("c0", "p0");
  b0.CreatePort("c1", "p1");
  //b0.SetPortMaxConnection("p", 2);
  
  // fork
  Box b1 = b0.Fork("b1");
  b1.CreateChannel("c2", "Channel"); // would be merged with c1
  b1.ConnectNodeToChannel("n1", "c2"); // n1 connect to c2
  b1.CreatePort("c2", "p2");
  b1.SetPortMaxConnection("p2", 2);
  b1.ConnectPort("p1", b1, "p2"); // internal connection of "p1-to-p2" in "b1", TODO to be check
  b0.ConnectPort("p0", b1, "p0");
  b0.ConnectPort("p1", b1, "p2");

  // builder instanciate
  NsomBuilder builder("test-net");
  builder.AddBox(b0);
  builder.AddBox(b1);
  cout << builder.Build() << endl;
}

void box_multiconnection() {
  // create geteway
  Box b0("b0", "Box");
  b0.CreateNode("n0", "Node");
  b0.CreateChannel("c0", "Channel");
  b0.ConnectNodeToChannel("n0", "c0");
  b0.CreatePort("c0", "p0")
    .SetMaxConnection(10);
  
  vector<Box> boxs;
  for (int i = 0; i<10; ++i) {
    string name = string{} + "b" + std::to_string(i);
    Box b(name, "Box");
    b.CreateNode("n0", "Node");
    b.CreateChannel("c0", "Channel");
    b.ConnectNodeToChannel("n0", "c0");
    b.CreatePort("c0", "p0");
    // FIXME multi-connection
    b.ConnectPort("p0", b0, "p0");
    boxs.push_back(b);
  }

  // builder instanciate
  NsomBuilder builder("test-net");
  builder.AddBox(b0);
  for (const auto& b : boxs) {
    builder.AddBox(b);
  }
  cout << builder.Build() << endl;
}

void bridge_test() {
  /* create switch-box */
  Box b0("bridge", "SwitchBox");
  b0.CreateNode("n0", "Node");
  // channel creation and configuration for bridge
  b0.CreateChannel("c0", "Csma")
    .SetConfig({ {"Address", { {"Base", "192.168.10.1/24"}, {"Type", "NetworkUnique"} }}});
  b0.CopyChannel("c0", "c1");
  b0.CopyChannel("c0", "c2");
  b0.TriConnect({"Switch"}, "n0", "c0", "p0");
  b0.TriConnect({"Switch"}, "n0", "c1", "p1");
  b0.TriConnect({"Switch"}, "n0", "c2", "p2");
  
  /* create terminal-box */
  Box b1("b1", "TerminalBox");
  b1.CreateNode("n0", "Node");
  b1.CreateChannel("c0", "Channel");
  b1.TriConnect("n0", "c0", "p0");
  Box b2 = b1.Fork("b2");
  Box b3 = b1.Fork("b3");

  // set points
  b0.SetPoint({10,10});
  b1.SetPoint({10,0});
  b2.SetPoint({0,20});
  b3.SetPoint({20,20});

  // connect box
  b1.ConnectPort("p0", b0, "p0");
  b2.ConnectPort("p0", b0, "p1");
  b3.ConnectPort("p0", b0, "p2");

  // NSOM build
  NsomBuilder builder("TestNet");
  builder.AddBox(b0);
  builder.AddBox(b1);
  builder.AddBox(b2);
  builder.AddBox(b3);
  cout << builder.Build() << endl;
}
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

void box_order_test() {

  init();
  
  {
    cout << "- Incomparable BoxType, Equality ParentChild" << endl;
    cout << "  (-,Parent), (-,Parent)" << endl;
    BoxPriority a{"", ParentChild::Parent};
    BoxPriority b{"", ParentChild::Parent};
    if (a > b) { cout << ">"; } else if(a < b) { cout << "<"; }
    if (a == b) { cout << "=" << endl; } else { cout << "!=" << endl; }
  }
  {
    cout << "- Equality BoxType, Equality ParentChild" << endl;
    cout << "  (Terminal,Parent), (Terminal,Parent)" << endl;
    BoxPriority a{"Terminal", ParentChild::Parent};
    BoxPriority b{"Terminal", ParentChild::Parent};
    if (a > b) { cout << ">"; } else if(a < b) { cout << "<"; }
    if (a == b) { cout << "=" << endl; } else { cout << "!=" << endl; }
  }
  {
    cout << "- Comparable BoxType, Equality ParentChild" << endl;
    cout << "  (Terminal,Parent), (Switch,Parent)" << endl;
    BoxPriority a{"Terminal", ParentChild::Parent};
    BoxPriority b{"Switch", ParentChild::Parent};
    if (a > b) { cout << ">"; } else if(a < b) { cout << "<"; }
    if (a == b) { cout << "=" << endl; } else { cout << "!=" << endl; }
  }
  {
    cout << "- Incomparable BoxType, Comparable ParentChild" << endl;
    cout << "  Parent, Child" << endl;
    BoxPriority a{"", ParentChild::Parent};
    BoxPriority b{"", ParentChild::Child};
    if (a > b) { cout << ">"; } else if(a < b) { cout << "<"; }
    if (a == b) { cout << "=" << endl; } else { cout << "!=" << endl; }
  }
  {
    cout << "- BoxType incomparable on one and not on the other, ParentChild is comparable" << endl;
    cout << "  (Terminal,Child), (-,Parent)" << endl;
    BoxPriority a{"Terminal", ParentChild::Child};
    BoxPriority b{"", ParentChild::Parent};
    if (a > b) { cout << ">"; } else if(a < b) { cout << "<"; }
    if (a == b) { cout << "=" << endl; } else { cout << "!=" << endl; }
  }
  {
    cout << "- BoxType incomparable on one and not on the other, ParentChild is comparable" << endl;
    cout << "  (-,Child), (Terminal,Parent)" << endl;
    BoxPriority a{"", ParentChild::Child};
    BoxPriority b{"Terminal", ParentChild::Parent};
    if (a > b) { cout << ">"; } else if(a < b) { cout << "<"; }
    if (a == b) { cout << "=" << endl; } else { cout << "!=" << endl; }
  }
}

void test() {
  //box_self_port_connect_test();
  //box_multiconnection();
  //bridge_test();
  bridge_test2(); // TODO implement type comparison, Now Working!!
  //box_order_test();
}

int main() {
  test();
  return 0;
}
