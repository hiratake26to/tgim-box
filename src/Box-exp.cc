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

struct Node {
  string name;
  string type;

  string ToString() const {
    std::stringstream ss;
    ss << "Node`" << name << ":" << type << "`";
    return ss.str();
  }
};

struct Channel {
  string name;
  string type;
  bool is_used;

  string ToString() const {
    std::stringstream ss;
    ss << "Channel`" << name << ":" << type << "`";
    ss << "{used:"<<(is_used ? "1":"0")<<"}";
    return ss.str();
  }
};

struct Port {
  // read only
  string from_channel_type;
  string to_box;
  string to_box_channel_type;

  int relation;  // 0:none, 1:parent-child
  int is_parent; // 1: parent, 0:child

  // !!EXPERIMENTAL PROPERTY!!
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

    throw std::logic_error("invalid Port relation");
  }

  bool IsParent() const {
    return (is_parent ? true : false);
  }

  string ToString() const {
    std::stringstream ss;
    ss << "Port{";
    ss << from_channel_type << ",";
    ss << to_box << ",";
    ss << to_box_channel_type << ",";
    ss << relation;
    ss << "}";

    return ss.str();
  }
};

class Box {
private:
  // [TODO] 名前をポインタとして使ってしまっているので, 別途ポインタを用意したい.
  string name;
  string type;

public:
  Box(string name, string type): name(name), type(type) {
    // initialization
  }

  // [TODO]
  // 現実装では, 名前の変更は接続処理の前でなければならない.
  // 名前はポインタのように扱われるため, 接続後に名前を変更した場合, 動作は未定義.
  //
  // 接続後の名前の変更を禁止にするアイディアはどうだろうか.
  void SetName(string name) {
    this->name = name;
  }

  void SetType(string type) {
    this->type = type;
  }

  string GetName() const {
    return this->name;
  }

  string GetType() const {
    return this->type;
  }

  Box Fork(string name) const {
    Box ret = *this;
    ret.SetName(name);
    return ret;
  }

  void AddNode(string name, string type) {
    nodes.push_back(Node{name, type});
  }

  void AddChannel(string name, string type) {
    channels.push_back(Channel{name, type});
  }

  optional<std::reference_wrapper<Channel>> FindChannel(string name) {
    for (size_t i = 0; i < channels.size(); ++i) {
      if (channels[i].name == name) return channels[i];
    }
    return {};
  }

  optional<std::reference_wrapper<Node>> FindNode(string name) {
    for (size_t i = 0; i < nodes.size(); ++i) {
      if (nodes[i].name == name) return nodes[i];
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

  void Connect(const string& from_this_channel_name, Box& to_box, const string& to_box_channel_name) {
    Channel& from_channel = this->FindChannel(std::move(from_this_channel_name)).value();
    Channel& to_channel = to_box.FindChannel(std::move(to_box_channel_name)).value();

    if (auto msg = this->CheckInvalidChannel(from_channel, to_channel)) {
      std::cerr << "connection failed. object dump:" << endl;
      std::cerr << "from box: " << this->ToString() << endl;
      std::cerr << "to box: " << to_box.ToString() << endl;
      std::cerr << "checker message: " << msg.value() << endl;
      throw std::logic_error("not pass through check a channel");
    }

    if (this->type == to_box.type && from_channel.type == to_channel.type) {
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

  string ToString() const {
    std::stringstream ss;

    ss << "Box`";
    ss << name << ":" << type;
    ss << "`{ ";

    ss << "nodes: [ ";
    for (const auto& n: nodes) {
      ss << n.ToString() << " ";
    }
    ss << "] ";

    ss << "channels: [ ";
    for (const auto& c: channels) {
      ss << c.ToString() << " ";
    }
    ss << "] ";

    ss << "ports: [ ";
    for (const auto& p: ports) {
      ss << p.ToString() << " ";
    }
    ss << "] ";

    ss << "}";

    return ss.str();
  }

  vector<Node> nodes;
  vector<Channel> channels;
  vector<Port> ports; // ports is channels which has used by box connecting.
};

class NsomBuilder {
private:

public:
  NsomBuilder() {};

  void AddBox(Box box);

  string Build();
};


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

void test() {
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
}

int main() {
  test();
  return 0;
}
