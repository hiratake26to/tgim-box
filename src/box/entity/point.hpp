struct Point {
  int x;
  int y;
  optional<int> z;

  Point operator +(const Point& rhs) const {
    Point ret{
      .x = this->x + rhs.x,
      .y = this->y + rhs.y
    };
    if (this->z && rhs.z) {
      ret.z = this->z.value() + rhs.z.value();
    }
    return ret;
  }
};
