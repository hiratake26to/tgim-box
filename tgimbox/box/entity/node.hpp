/*
The MIT License

Copyright 2019 hiratake26to

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
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
