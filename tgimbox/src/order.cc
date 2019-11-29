#include "common.hpp"
#include "../box/order.hpp"

namespace tgim {

map<string,int> map_boxtype_num;

void boxtype_append(string str) {
  map_boxtype_num[str] = map_boxtype_num.size();
}
void init() {
  // the smaller value, the higher priority.
  // descending order.
  //boxtype_append("Internet");
  //boxtype_append("Subnet");
  //boxtype_append("Router");
  boxtype_append("Switch");
  //boxtype_append("Gateway");
  //boxtype_append("Hub");
  boxtype_append("Terminal");
}

optional<int> boxtype_to_num(string str) {
  if (map_boxtype_num.find(str) == map_boxtype_num.end()) return {};
  return map_boxtype_num.at(str);
}

optional<BoxEquality> CompBoxType(string lhs, string rhs) {
  auto lret = boxtype_to_num(lhs);
  auto rret = boxtype_to_num(rhs);
  if (not (lret and rret)) {
    return {};
    //throw std::logic_error("Exception: could not compare box type, due to no ordering box type!");
  }
  auto lval = lret.value();
  auto rval = rret.value();
  if (lval == rval) return BoxOrdering::EQ;
  else if (lval < rval) return BoxOrdering::LT;
  else if (lval > rval) return BoxOrdering::GT;

  return {};
}

optional<BoxEquality> CompBoxParentChild(ParentChild lhs, ParentChild rhs) {
  if (lhs == rhs) return BoxOrdering::EQ;
  else if (lhs == ParentChild::Parent) return BoxOrdering::LT;
  else if (rhs == ParentChild::Parent) return BoxOrdering::GT;
  return {};
}

BoxEquality CompBoxPriority(const BoxPriority& lhs, const BoxPriority& rhs) {
  if (auto result = CompBoxType(lhs.box_type, rhs.box_type)) {
    return result.value();
  }
  if (auto result = CompBoxParentChild(lhs.parent_child, rhs.parent_child)) {
    return result.value();
  }

  throw std::logic_error("Exception: could not compare box prent-child relation!");
}

bool operator<(const BoxPriority& lhs, const BoxPriority& rhs) {
  if (CompBoxPriority(lhs, rhs) == BoxOrdering::LT) {
    return true;
  }
  else if (CompBoxPriority(lhs, rhs) == BoxOrdering::GT) {
    return false;
  }
  return false;
}
bool operator> (const BoxPriority& lhs, const BoxPriority& rhs){ return rhs < lhs; }
bool operator<=(const BoxPriority& lhs, const BoxPriority& rhs){ return !(lhs > rhs); }
bool operator>=(const BoxPriority& lhs, const BoxPriority& rhs){ return !(lhs < rhs); }
bool operator==(const BoxPriority& lhs, const BoxPriority& rhs) {
  if (CompBoxPriority(lhs, rhs) == BoxOrdering::EQ) {
    //if (lhs.ordering_type == OrderingType::BoxType) {
    //  std::cerr << "warning: no ordering box type!" << endl;
    //}
    //else if (lhs.ordering_type == OrderingType::ParentChild) {
    //  std::cerr << "warning: no parent-child relation!" << endl;
    //}
    return true;
  }
  return false;
}
bool operator!=(const BoxPriority& lhs, const BoxPriority& rhs){ return !(lhs == rhs); }

}
