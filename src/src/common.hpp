#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <functional>
#include <algorithm>
#include <utility>
#include <map>
#include <typeinfo>
#include <any>
#include <variant>
#include <boost/core/demangle.hpp>

using std::cout;
using std::endl;
using std::optional;
using std::string;
using std::vector; // should not use reference to a element of vector.
using std::list; // allow to use reference of a element.
using std::pair;
using std::map;
using std::type_info;
using std::any;

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../schedule/schedule.hpp"

#define RANGE(v) v.begin(), v.end()
#define UNIQUE(v) {\
  std::sort(RANGE(v));\
  auto last = std::unique(RANGE(v));\
  v.erase(last, v.end());\
}
