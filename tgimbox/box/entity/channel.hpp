/*
The MIT License

Copyright 2019 hiratake26to

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once

struct Channel {
  string name;
  string type;
  //optional<string> port; // which port use for this channel
  list<string> ports; // which port use for this channel
  json config;
  optional<string> tag;

  Channel& SetConfig(json val) {
    config = val;
    return *this;
  }

  string ToString(int level=0) const {
    std::stringstream ss;
    ss << "Channel`" << name << ":" << type << "`";
    if (level >= 1) {
      //ss << "{port:" << port.value_or("");
      ss << "{ports:[";
      for (const auto& p : ports) {
        ss << p << ",";
      }
      ss << "]";
      ss << ",conf:" << config;
      if (tag) {
        ss << ",tag:" << tag.value();
      }
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
    ss << "MChannel{" << value.ToString();
    ss << "}";
    return ss.str();
  }
};
