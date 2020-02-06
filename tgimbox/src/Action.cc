/*
The MIT License

Copyright 2019 hiratake26to

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "common.hpp"
#include "../schedule/schedule.hpp"

namespace tgim {

//////////////////////////////////////////////////
// PrimitiveAction (ActionType, AppType)

string ActionToString(const Action& act, int l) {
  std::stringstream ss;
  ss << "Action{";
  if (const auto& val = std::get_if<PrimitiveAction>(&act)) {
    ss << val->ToString();
  } else if (const auto& val = std::get_if<Sig>(&act)) {
    ss << val->ToString();
  } else if (const auto& val = std::get_if<ScheduleRef>(&act)) {
    ss << endl;
    ss << val->get().ToString(l);
  } else {
    throw std::logic_error("could not action to string!");
  }
  ss << "}";
  return ss.str();
}

//////////////////////////////////////////////////
// PrimitiveAction (ActionType, AppType)

PrimitiveAction::PrimitiveAction(string type, json param)
: type(type), param(param)
{
  if ( not(param.is_null() or param.is_object()) ){
    std::stringstream ss;
    ss << "`params` of action must be json that ether null or object.";
    throw std::logic_error(ss.str());
  }
}
string PrimitiveAction::ToString() const {
  std::stringstream ss;
  ss << "PreAct{"
    << "type:" << type
    << ",param:" << param
    << "}";
  return ss.str();
}
bool PrimitiveAction::operator==(const PrimitiveAction& rhs) const {
  return (this->type == rhs.type and this->param == rhs.param);
}
bool PrimitiveAction::operator!=(const PrimitiveAction& rhs) const { return !(*this==rhs); }
bool PrimitiveAction::operator<(const PrimitiveAction& rhs) const {
  if (this->type < rhs.type) return true;
  if (this->param < rhs.param) return true;
  if (this->type > rhs.type) return false;
  if (this->param > rhs.param) return false;
  return false;
}
bool PrimitiveAction::operator> (const PrimitiveAction& rhs) const { return rhs < *this; }
bool PrimitiveAction::operator<=(const PrimitiveAction& rhs) const { return !(*this > rhs); }
bool PrimitiveAction::operator>=(const PrimitiveAction& rhs) const { return !(*this < rhs); }

}
