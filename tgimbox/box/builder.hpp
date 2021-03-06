/*
The MIT License

Copyright 2019 hiratake26to

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once

#include "box.hpp"
#include "order.hpp"

namespace tgim {

class NsomBuilder {
private:
  vector<Box> boxs_;
  string name_;
  optional<Box> global_sdl_;

public:
  NsomBuilder(string name)
  : name_(name) {}

  void AddBox(const Box& box) {
    boxs_.push_back(box);
  }

  void AddBox(const vector<Box>& boxs) {
    for (auto&& i : boxs) {
      boxs_.push_back(i);
    }
  }

  void SetGlobalSdl(const Box& box) {
    global_sdl_ = box;
  }

  // build ns-3 code
  string Build() {
    vector<Box> boxs = boxs_;
    for (auto&& b : boxs) {
      // name mangling to avoid NSOM item's name conflict.
      b.Mangle();
      // set global schedule
      if (global_sdl_) {
        b.Sdl(global_sdl_.value().GetSchedule());
      }

      //cout << "[DEBUG] " << b.ToString(100) << endl;
      //cout << "[DEBUG] " << b.DumpSchedule() << endl;
    }

    // TODO marge channel preprocess
    MergeChannel(boxs);

    json j;
    cout << "[builder] name generating..." << endl;
    j["name"]    = name_;
    cout << "[builder] node generating..." << endl;
    j["node"]    = GenNodeList(boxs);
    cout << "[builder] channel generating..." << endl;
    j["channel"] = GenChannelList(boxs);
    cout << "[builder] subnet generating..." << endl;
    j["subnet"]  = GenSubnetList();
    cout << "[builder] apps generating..." << endl;
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
      .channels = {from_channel.name, to_channel.name}
      //.value
    };

    // select merged channel
    //cout << "[DEBUG] parent_box " << parent_box.ToString(1) << endl;
    //cout << "[DEBUG] child_box " << child_box.ToString(1) << endl;
    BoxPriority bp_parent = ToBoxPriority(parent_box,parent_port);
    BoxPriority bp_child = ToBoxPriority(child_box,child_box_port);
    if (bp_parent < bp_child) {
      ret.value = from_channel;
    } else if (bp_parent > bp_child) {
      ret.value = to_channel;
    } else {
      throw std::logic_error("invalid port relation");
    }

    // overwrite name to merged channel name
    ret.value.name = ss.str();

    return ret;
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
        cout << "[box] MerCh: " << mch.value.ToString(1) << endl;
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
        //if (not ch.port) j[ch.name] = GenChannelBody(ch);
        if (0 == ch.ports.size()) j[ch.name] = GenChannelBody(ch);
      }
    }

    return j;
  }

  json GenChannelBody(Channel channel) {
    json j;
    j["type"] = channel.type;
    if (channel.tag) j["tag"] = channel.tag.value();
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
        //cout << "[DEBUG] " << task.ToString() << endl;
        string task_name = box.GetName() + "_T" + std::to_string(task_i);
        j[task_name] = GenAppBody(box, task);
        ++task_i;
      }
    }

    return j;
  }
  json GenAppBody(const Box& box, const Task& task) {
    json j;

    if (auto main_box = box.FindNodeFromType("Main")) {
      j["install"] = main_box.value().get().name;
    } else {
      std::stringstream ss;
      ss << "exception: " << box.ToString() << " have no Main-type node!\n";
      ss << "\tmake node has `Main` type in the box";
      throw std::runtime_error(ss.str());
    }

    try {
      const auto& action = std::get<PrimitiveAction>(task.action);
      j["type"] = action.type;
      j["args"] = action.param;
    } catch (const std::bad_variant_access&) {
      std::stringstream ss;
      ss << "exception: task.action is not type of PrimitiveAction!\n";
      std::visit([&ss](auto&& arg) {
          using T = std::decay_t<decltype(arg)>;
          ss << "\ttask.action type: " << boost::core::demangle(typeid(T).name());
          }, task.action);
      throw std::logic_error(ss.str());
    }

    j["nodes"] = {};
    for (const auto& node: box.nodes) {
      j["nodes"] += node.name;
    }
    j["channels"] = {};
    for (const auto& ch: box.channels) {
      j["channels"] += ch.name;
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

}
