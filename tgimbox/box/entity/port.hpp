/*
The MIT License

Copyright 2019 hiratake26to

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once

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
