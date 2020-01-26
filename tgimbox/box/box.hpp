#pragma once

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
#include "../schedule/schedule.hpp"

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

#include "entity/entity.hpp"

namespace tgim {
// box
class Box {
private:
  // [TODO] 名前をポインタとして使ってしまっているので, 別途ポインタを用意する?
  string name_;
  string type_;
  Point point_;
  Schedule schedule_;

public:
  // NOTE mutable is meaning that allow returning reference from 'const function'
  mutable list<Node> nodes;
  mutable list<Channel> channels;
  mutable list<Port2> ports; // ports is channels which has used by box connecting.

public:
  Box(string name, string type);
  Box& SetName(string name);
  Box& SetType(string type);
  Box& SetPoint(Point pt);
  string GetName() const;
  string GetType() const;
  vector<string> GetPorts() const;
  Point GetPoint() const;
  string AsHost() const;
  //int PortNum(string) const;
  Box Fork(string name) const;
  Box Fork(string name, string type) const;
  Channel& CopyChannel(string src, string dst);
  Channel& CreateChannel(string name, string type);
  Node& CopyNode(string src, string dst);
  Node& CreateNode(string name, string type);
  Port2& CreatePort(const string& from_this_channel_name, const string& port_name);
  void ConnectNodeToChannel(const string& node_name, const string& channel_name, const vector<string>& roles = {});
  void ConnectPort(const string& from_this_port_name, Box& to_box, const string& to_box_port_name);
  void SetPortMaxConnection(const string& port_name, int num);
  void TriConnect(string node_name, string channel_name, string port_name);
  void TriConnect(vector<string> node_roles, string node_name, string channel_name, string port_name);

  optional<std::reference_wrapper<Channel>> FindChannel(string name) const;
  optional<std::reference_wrapper<Node>> FindNodeFromType(string type) const;
  optional<std::reference_wrapper<Node>> FindNode(string name) const;
  optional<std::reference_wrapper<Port2>> FindPort2(string name) const;

  //optional<string> ResolveConn(string channel_name) const;
  vector<string> ResolveConn(string channel_name) const;

  ScheduleControllBlock At(EventSpecifer es);
  ScheduleControllBlock Sdl2();
  ScheduleControllBlock Sdl(const Schedule& sdl);
  Schedule GetSchedule() const;
  string DumpSchedule() const;
  string ToString(int level=0) const;
  vector<Task> FlattenSchedule() const;
  void ClearSchedule();

  vector<Netif> GenNodeConnList(const Node& node) const;
  void Mangle();

private:
  vector<Event> SearchPrimitiveEvents(const vector<Task>& sdl, const Task& task) const;
  static vector<Event> SearchPrimitiveEvents(const vector<Task>& sdl, const Task& task, vector<Task>& breadcrumb);
  optional<string> CheckInvalidChannel(const Channel& from_channel, const Channel& to_channel);
  // not recommendation for user to use
  void AddTask(Event evt, Action action);
  vector<Task> FlattenSchedule(const Schedule& sdl,int callid=0) const;
  string PrefixBox(string name) const;
  string PrefixNode(string name) const;
  string PrefixChannel(string name) const;
  string PrefixPort(string name) const;
  string PrefixTag(string name) const;
};
}
