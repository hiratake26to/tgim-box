/*
The MIT License

Copyright 2019 hiratake26to

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
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

#define RANGE(v) v.begin(), v.end()
#define UNIQUE(v) {\
  std::sort(RANGE(v));\
  auto last = std::unique(RANGE(v));\
  v.erase(last, v.end());\
}
