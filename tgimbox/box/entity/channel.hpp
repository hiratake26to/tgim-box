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
