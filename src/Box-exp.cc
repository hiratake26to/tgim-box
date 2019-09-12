#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include <functional>

using std::cout;
using std::endl;
using std::optional;
using std::string;
using std::vector;

#include <nlohmann/json.hpp>
using json = nlohmann::json;


//////////////////////////////
#define PORT2 1
//////////////////////////////


struct Netif {
  string connect;
  optional<string> as;
};

struct Point {
  int x;
  int y;
  optional<int> z;
};

struct Node {
  string name;
  string type;
  // connection for internal node of box
  optional<string> connect_to; // connecting a channel
  optional<string> role; // connecting as a role
  bool is_connected;
  Point point;
  

  void Connect(string channel_name, optional<string> role_name) {
    if (is_connected) {
      std::stringstream ss;
      ss << "exception: the node already has connected to a channel." << endl;
      ss << "\tnode dump: " << ToString(1) << endl;
      throw std::logic_error(ss.str());
    }

    this->connect_to = channel_name;
    this->role = role_name;
    is_connected = true;
  }

  void Connect(string channel_name) {
    return Connect(channel_name, {});
  }

  // list of connections, from node to channel
  vector<Netif> GenConnList() const {
    vector<Netif> list;
    if (connect_to) {
      list.push_back({connect_to.value(), role});
    }

    return list;
  }

  string ToString(int level=0) const {
    std::stringstream ss;
    ss << "Node`" << name << ":" << type << "`";
    if (level >= 1) {
      ss << "{to:" << connect_to.value_or("");
      ss << ",as:" << role.value_or("");
      ss << "," << is_connected;
      ss << "}";
    }
    return ss.str();
  }
};

struct Channel {
  string name;
  string type;
  bool is_used; // if the flag is 1, channel has used for box ports (this is called portalized)
  optional<string> port; // which port use this channel

  string ToString(int level=0) const {
    std::stringstream ss;
    ss << "Channel`" << name << ":" << type << "`";
    if (level >= 1) {
      ss << "{used:"<<(is_used ? "1":"0")<<",";
      ss << "port:" << port.value_or("");
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

  int relation;
  int is_parent;
  //int priority;

  int cur_conn = 0;
  int max_conn = 1;

  // NOTE merged channel temporary put in Port2
  optional<MergedChannel> mchannel;

  void IncCurConn() {
    if ( cur_conn > max_conn-1 ) {
      std::stringstream ss;
      ss << "exception: `cur_conn = " << cur_conn
         << "` increasing. due to over its maximum `max_conn = " << max_conn << "`.";
      ss << "\tport dump: " << ToString() << endl;
      throw std::logic_error(ss.str());
    }
    cur_conn++;
  }

  string ToString(int level=0) const {
    std::stringstream ss;
    ss << "Port2`" << name << "`{";
    ss << from_channel << ",";
    ss << to_box << ",";
    ss << to_box_port << ",";
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

public:
  Box(string name, string type): name_(name), type_(type) {
    // initialization
  }

  // [TODO]
  // 現実装では, 名前の変更は接続処理の前でなければならない.
  // 名前はポインタのように扱われるため, 接続後に名前を変更した場合, 動作は未定義.
  //
  // 接続後の名前の変更を禁止にするアイディアはどうだろうか.
  void SetName(string name) {
    this->name_ = name;
  }

  void SetType(string type) {
    this->type_ = type;
  }

  string GetName() const {
    return this->name_;
  }

  string GetType() const {
    return this->type_;
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
      if (node.connect_to) {
        node.connect_to.value() = this->name_ + PrefixChannel(node.connect_to.value());
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

  void AddNode(string name, string type) {
    nodes.push_back(Node{name, type});
  }

  void AddChannel(string name, string type) {
    channels.push_back(Channel{name, type});
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
    if (from_channel.is_used || to_channel.is_used) {
      std::stringstream ss;
      if (from_channel.is_used) ss << "from channel: " << from_channel.name << " is used. ";
      if (to_channel.is_used) ss << "to channel: " << to_channel.name << " is used. ";
      return ss.str();
    }
    return {};
  }

#if PORT2
  void CreatePort(const string& from_this_channel_name, const string& port_name) {
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

    from_channel.is_used = true;
    from_channel.port = port_name;
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

    from_port.to_box = to_box.GetName();
    from_port.to_box_port = to_box_port_name;
    from_port.relation = 0;
    from_port.is_parent = 0;

    to_port.to_box = this->GetName();
    to_port.to_box_port = from_this_port_name;
    to_port.relation = 0;
    to_port.is_parent = 1;

    // update current connection count
    try {
      from_port.IncCurConn();
      to_port.IncCurConn();
    } catch(const std::logic_error& e) {
      std::stringstream ss;
      ss << e.what() << endl;
      ss << "box dump: " << ToString() << endl;
      ss << "\tconnection error dut to increasing port connection count";
      throw std::logic_error(ss.str());
    }

  }
#else
  optional<bool> Connect(const string& from_this_channel_name, Box& to_box, const string& to_box_channel_name) {
    try {
      Connect_(from_this_channel_name, to_box, to_box_channel_name);
    } catch (const std::bad_optional_access& e) {
      //throw std::logic_error("could not find channel in box");
      return {};
    }
    return true;
  }

  void Connect_(const string& from_this_channel_name, Box& to_box, const string& to_box_channel_name) {
    Channel& from_channel = this->FindChannel(std::move(from_this_channel_name)).value();
    Channel& to_channel = to_box.FindChannel(std::move(to_box_channel_name)).value();

    if (auto msg = this->CheckInvalidChannel(from_channel, to_channel)) {
      std::stringstream ss;
      ss << "connection failed. object dump:" << endl;
      ss << "\tfrom box: " << this->ToString() << endl;
      ss << "\tto box: " << to_box.ToString() << endl;
      ss << "\tchecker message: " << msg.value() << endl;
      ss << "\tnot pass through check a channel";
      throw std::logic_error(ss.str());
    }

    if (this->type_ == to_box.type_ && from_channel.type == to_channel.type) {
      ports.push_back(
          Port{from_channel.type, to_box.GetName(), to_channel.type, 0, 0} );
      to_box.ports.push_back(
          Port{to_channel.type, this->GetName(), from_channel.type, 0, 1} );
    } else {
      ports.push_back(
          Port{from_channel.type, to_box.GetName(), to_channel.type, 1, 0} );
      to_box.ports.push_back(
          Port{to_channel.type, this->GetName(), from_channel.type, 1, 1} );
    }
    from_channel.is_used = true;
    to_channel.is_used = true;

  }
#endif

  void InternallyConnect(const string& node_name, const string& channel_name) {
    Node& node = this->FindNode(std::move(node_name)).value();
    //Channel& channel = this->FindChannel(std::move(channel_name)).value();

    try {
      node.Connect(channel_name);
    } catch(const std::logic_error& e) {
      std::stringstream ss;
      ss << e.what() << endl;
      ss << "box dump: " << ToString() << endl;
      ss << "\tinternally connecting error";
      throw std::logic_error(ss.str());
    }
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
#if PORT2
  mutable vector<Port2> ports; // ports is channels which has used by box connecting.
#else
  vector<Port> ports; // ports is channels which has used by box connecting.
#endif
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
    for (auto&& b : boxs) b.Mangle();

#if PORT2
    // TODO marge channel preprocess
    MergeChannel(boxs);
#endif

    json j;
    j["name"]    = name_;
    j["node"]    = GenNodeList(boxs);
    j["channel"] = GenChannelList(boxs);
    j["subnet"]  = GenSubnetList();
    j["apps"]    = GenAppsList();

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

  // これいる?
  optional<std::reference_wrapper<Port2>> FindRelevantPort2(Port2 port, vector<Box>& boxs) {
    Box& to_box = FindBox(port.to_box, boxs).value();
    return to_box.FindPort2(port.to_box_port).value();
  }

  MergedChannel GetMergeChannel(vector<Box> boxs, Box from_box, Port2 from_port) {
    // from_box.port.from_channel -> from_box.channel
    Channel& from_channel = from_box.FindChannel(from_port.from_channel).value();
    // from_box.port.to_box_name -> to_box
    //   -> from_box.port.to_box_port_name -> to_box.port
    //   -> to_box.channel
    Box& to_box = FindBox(from_port.to_box, boxs).value();
    Port2& to_box_port = to_box.FindPort2(from_port.to_box_port).value();
    Channel& to_channel = to_box.FindChannel(to_box_port.from_channel).value();

    std::stringstream ss;
    ss << "_M"; // _M prefixed to a merged channel, as signature
    ss << from_box.GetName();
    ss << from_port.name;
    ss << to_box.GetName();
    ss << to_box_port.name;

    MergedChannel ret = {
      .channels = {from_channel.name, to_channel.name},
      .value = {ss.str(), ""}
    };

    // チャネルの融合
    // 対等な接続なのでどれを取ってもよい
    if (to_box_port.relation == 0) {
      ret.value.type = from_channel.type;
      return ret;
    }
    
    // 親子関係があるので親から取る
    if (to_box_port.is_parent == 1) {
      ret.value.type = from_channel.type;
      return ret;
    } else {
      ret.value.type = to_channel.type;
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
    auto pt = node.point;
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
    if (conn.as) netif["as"] = conn.as.value();

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
#if PORT2
          // TODO
          //Channel ch = GetMergeChannel(boxs, box, port);
          Channel ch = port.mchannel.value().value;
          j[ch.name] = GenChannelBody(ch);
#else
          Channel ch = port.GetMergeChannel();
          j[ch.name] = GenChannelBody(ch);
#endif
        }
      }
    }
    for (const auto& box : boxs) {
      for (const auto& ch : box.channels) {
        // set a node's name to a key,
        // the node's body create, and set
        // TODO
        if (not ch.is_used) j[ch.name] = GenChannelBody(ch);
      }
    }

    return j;
  }

  json GenChannelBody(Channel channel) {
    json j;
    j["type"] = channel.type;

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

#if not PORT2

void box_test() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  // create basic box
  Box basic_box("undefined", "Basic");
  basic_box.AddNode("N0", "NodeType");
  basic_box.AddChannel("C0", "ChannelType");

  // create b0, b1, b2 from basic box
  Box b0 = basic_box.Fork("b0");

  Box b1 = basic_box.Fork("b1");
  b1.AddChannel("C1", "ChannelType");

  Box b2 = basic_box.Fork("b2");

  // connect:
  // b0[C0] - [C0]b1[C1] - [C0]b2
  b0.Connect("C0", b1, "C0");
  b1.Connect("C1", b2, "C0");

  cout << b0.ToString() << endl;
  cout << b1.ToString() << endl;
  cout << b2.ToString() << endl;
}

void box_channel_merge_test() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  // create basic box
  Box basic_box("undefined", "Basic");
  basic_box.AddNode("R", "Router");
  basic_box.AddChannel("C", "Csma");

  // create router box
  Box r0 = basic_box.Fork("Router0");
    r0.AddChannel("C1", "Csma");
  Box r1 = basic_box.Fork("Router1");
  Box r2 = basic_box.Fork("Router2");

  // connect:
  // r0[C] -> [C]r1
  //     | -> [C]r2
  r0.Connect("C", r1, "C");
  r0.Connect("C1", r2, "C");

  cout << r0.ToString() << endl;
  cout << r1.ToString() << endl;
  cout << r2.ToString() << endl;

  //
  vector<Box> boxs;
  boxs.push_back(r0);
  boxs.push_back(r1);
  boxs.push_back(r2);

  for (const auto& b : boxs)
  for (const auto& p : b.ports) {
    if (not p.IsParent()) {
      cout << b.GetName();
      cout << " connect to ";
      cout << p.to_box;
      cout << " via Channel type of ";
      cout << p.GetMergeChannelType() << endl;
    }
  }

}

void box_channel_merge_test2() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  // create basic box
  Box basic_box("undefined", "Basic");
  basic_box.AddNode("R", "Router");
  basic_box.AddChannel("C", "Csma");

  // create router box
  Box r0 = basic_box.Fork("Router0");
    r0.channels[0].type = "PPP";
    r0.AddChannel("C1", "PPP");
    r0.AddChannel("C2", "PPP");
  Box r1 = basic_box.Fork("Router1");
    r1.channels[0].type = "Csma";
  Box r2 = basic_box.Fork("Router2");
    r2.channels[0].type = "Csma";
  Box r3 = basic_box.Fork("Router3");
    r3.channels[0].type = "Csma";

  // connect:
  // r3[C] -> r0[C] -> [C]r1
  //              | -> [C]r2
  r0.Connect("C", r1, "C");
  r0.Connect("C1", r2, "C");
  r3.Connect("C", r0, "C2");

  cout << r0.ToString() << endl;
  cout << r1.ToString() << endl;
  cout << r2.ToString() << endl;

  //
  vector<Box> boxs;
  boxs.push_back(r0);
  boxs.push_back(r1);
  boxs.push_back(r2);
  boxs.push_back(r3);

  for (const auto& b : boxs)
  for (const auto& p : b.ports) {
    if (not p.IsParent()) {
      cout << b.GetName();
      cout << " connect to ";
      cout << p.to_box;
      cout << " via Channel type of ";
      cout << p.GetMergeChannelType() << endl;
    }
  }

}

void box_channel_merge_test3() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  // create basic box
  Box basic_box("undefined", "Pc");
  basic_box.AddNode("P", "Pc");
  basic_box.AddChannel("P", "PointToPoint");

  // create pc box
  Box p0 = basic_box.Fork("rb0");
  Box p1 = basic_box.Fork("rb1");
  Box p2 = basic_box.Fork("rb2");
  // create hub box
  Box h0("hb0", "Hub");
  h0.AddNode("H", "Hub");
  h0.AddChannel("P0", "PointToPoint");
  h0.AddChannel("P1", "PointToPoint");
  h0.AddChannel("P2", "PointToPoint");

  // connect:
  // r0[P] -> [P0]h0
  // r1[P] -> [P1]h0
  // r2[P] -> [P2]h0
  p0.Connect("P", h0, "P0");
  p1.Connect("P", h0, "P1");
  p2.Connect("P", h0, "P2");

  cout << p0.ToString() << endl;
  cout << p1.ToString() << endl;
  cout << p2.ToString() << endl;

  //
  vector<Box> boxs;
  boxs.push_back(p0);
  boxs.push_back(p1);
  boxs.push_back(p2);
  boxs.push_back(h0);

  // check PPP contract
  for (const auto& b : boxs)
  for (const auto& p : b.ports) {
    if (p.IsParent()) {
      //
    }
  }


  for (const auto& b : boxs)
  for (const auto& p : b.ports) {
    if (not p.IsParent()) {
      cout << b.GetName();
      cout << " connect to ";
      cout << p.to_box;
      cout << " via Channel type of ";
      cout << p.GetMergeChannelType() << endl;
    }
  }

}

void box_channel_merge_test4() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  Box dev0("dev0", "Device");
  dev0.AddNode("n", "Basic");
  dev0.AddChannel("c", "PointToPoint");
  Box dev1 = dev0.Fork("dev1");

  Box rt("rt", "Router");
  rt.AddNode("n", "Router");
  rt.AddChannel("c0", "Csma");
  rt.AddChannel("c1", "Csma");

  // dev0[c] -> [c0]rt
  dev0.Connect("c", rt, "c0");
  // dev1[c] -> [c1]rt
  dev1.Connect("c", rt, "c1");

  vector<Box> boxs;
  boxs.push_back(dev0);
  boxs.push_back(dev1);
  boxs.push_back(rt);
  for (const auto& b : boxs)
    for (const auto& p : b.ports) {
      if (not p.IsParent()) {
        cout << b.GetName();
        cout << " connect to ";
        cout << p.to_box;
        cout << " via Channel type of ";
        cout << p.GetMergeChannelType() << endl;
      }
    }

}

void box_channel_merge_test5() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  Box dev0("dev0", "Device");
  dev0.AddNode("n", "Basic");
  dev0.AddChannel("c", "PointToPoint");
  Box dev1 = dev0.Fork("dev1");

  // dev0[c] -> [c0]rt
  dev0.Connect("c", dev1, "c");

  vector<Box> boxs;
  boxs.push_back(dev0);
  boxs.push_back(dev1);
  for (const auto& b : boxs)
    for (const auto& p : b.ports) {
      if (not p.IsParent()) {
        cout << b.GetName();
        cout << " connect to ";
        cout << p.to_box;
        cout << " via Channel type of ";
        cout << p.GetMergeChannelType() << endl;
      }
    }

}

void internally_connect_test() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  Box b("box", "Box");
  b.AddNode("n0", "Node");
  b.AddNode("n1", "Node");
  b.AddChannel("c", "Channel");
  // A node connection to a channel is once.
  b.InternallyConnect("n0", "c");
  //b.InternallyConnect("n0", "c"); // the error occurs by that call it continue from above calling
  // but channel can accept connection from other nodes.
  b.InternallyConnect("n1", "c");

  // [TODO] investigate grouping channel.
  // otherwise, tagged channel is usable.
  //b.Tag("c0", "C");
  //b.Tag("c1", "C");
  //b.Tag("c2", "C");
  //b.MakeGroup({"c0", "c1", "c2"}, "C");
}

void all_box_test() {
  box_test();
  cout << endl;
  box_channel_merge_test();
  cout << endl;
  box_channel_merge_test2();
  cout << endl;
  box_channel_merge_test3();
  cout << endl;
  box_channel_merge_test4();
  cout << endl;
  box_channel_merge_test5();
  cout << endl;
  internally_connect_test();
  cout << "All test OK" << endl;
}

void builder_test() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  // create box
  Box b("box", "Box");
  b.AddNode("n0", "Node");
  b.AddNode("n1", "Node");
  b.AddChannel("c", "Channel");

  // internally connect
  b.InternallyConnect("n0", "c");
  b.InternallyConnect("n1", "c");

  Box b1 = b.Fork("box1");

  // builder instanciate
  NsomBuilder builder("test-net");
  builder.AddBox(b);
  builder.AddBox(b1);
  cout << builder.Build() << endl;
}

void builder_box_connection_test() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  // create box
  Box b("box", "Box");
  b.AddNode("n0", "Node");
  b.AddNode("n1", "Node");
  b.AddChannel("c", "Channel");

  // internally connect
  b.InternallyConnect("n0", "c");
  b.InternallyConnect("n1", "c");

  Box b1 = b.Fork("box1");

  // set point of node
  b.FindNode("n0").value().get().point = {.x=10, .y=20};
  b.FindNode("n1").value().get().point = {.x=20, .y=30};
  b1.FindNode("n0").value().get().point = {.x=110, .y=120};
  b1.FindNode("n1").value().get().point = {.x=120, .y=130};

  b.Connect("c", b1, "c").value();
  //b.Connect("c", b1, "c").value();

  // builder instanciate
  NsomBuilder builder("test-net");
  builder.AddBox(b);
  builder.AddBox(b1);
  cout << builder.Build() << endl;
}

#endif

void merge_channel_test() {
  // create box
  Box b0("b0", "Box");
  b0.AddNode("n", "Node");
  b0.AddChannel("c", "Channel");
  // internally connect
  b0.InternallyConnect("n", "c");

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
  b0.AddNode("n0", "Node");
  b0.AddNode("n1", "Node");
  b0.AddChannel("c0", "Channel");
  b0.AddChannel("c1", "Channel");
  // internally connect
  b0.InternallyConnect("n0", "c0");
  b0.InternallyConnect("n1", "c1");

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

void test() {
  multi_inner_node_test();
}

int main() {
  test();
  return 0;
}
