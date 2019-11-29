#pragma once

namespace tgim {
// Box型が順序付けされていれば BoxType
// 順序付けされていなければ ParentChild
enum class OrderingType {
  BoxType,
  ParentChild
};

enum class ParentChild {
  Parent, Child
};
enum class BoxOrdering {
  EQ, LT, GT
};
typedef BoxOrdering BoxEquality;

struct BoxPriority {
  string box_type;
  ParentChild parent_child;
};

//void boxtype_append(string str);
//void init();

optional<int> boxtype_to_num(string str);

optional<BoxEquality> CompBoxType(string lhs, string rhs);

optional<BoxEquality> CompBoxParentChild(ParentChild lhs, ParentChild rhs);

BoxEquality CompBoxPriority(const BoxPriority& lhs, const BoxPriority& rhs);

bool operator<(const BoxPriority& lhs, const BoxPriority& rhs);
bool operator> (const BoxPriority& lhs, const BoxPriority& rhs);
bool operator<=(const BoxPriority& lhs, const BoxPriority& rhs);
bool operator>=(const BoxPriority& lhs, const BoxPriority& rhs);
bool operator==(const BoxPriority& lhs, const BoxPriority& rhs);
bool operator!=(const BoxPriority& lhs, const BoxPriority& rhs);

}
