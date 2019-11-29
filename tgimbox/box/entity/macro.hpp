#define RANGE(v) v.begin(), v.end()
#define UNIQUE(v) {\
  std::sort(RANGE(v));\
  auto last = std::unique(RANGE(v));\
  v.erase(last, v.end());\
}
