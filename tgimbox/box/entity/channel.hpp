#pragma once

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
