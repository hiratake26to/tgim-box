#pragma once

#include "macro.hpp"

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
