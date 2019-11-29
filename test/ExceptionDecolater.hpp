#pragma once

#include <string>
#include <boost/core/demangle.hpp>
using std::string;

namespace edc {

class HondaDecolate {
  string str;
public:
  HondaDecolate(const std::exception& e) {
    std::stringstream ss;
    ss <<
      "YOU LOSE'''\n"
      << e.what() << std::endl <<
      "'''の勝ち👊😁！\n"
      "なんで負けたか明日まで考えておいてください！"
      "そしたら何かが見えて来るはずです！\n"
      "ほな頂きます！勝負日1日1回！また明日🤗🤗🤗！\n";
    str = ss.str();
  }
  string what() const noexcept {
    return str;
  }
};
class HonhimaDecolate {
  string str;
public:
  HonhimaDecolate(const std::exception& e) {
    std::stringstream ss;
    ss << "もうわかんないよ…バーチャル `"
      << boost::core::demangle(typeid(e).name())
      << "` ってなんだー！？😣\n"
      << "<<🐻\n"
      << e.what() << std::endl
      << ">>ありがてぇてぇ🌻\n";
    str = ss.str();
  }
  string what() const noexcept {
    return str;
  }
};

}
