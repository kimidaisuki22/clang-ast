#include <filesystem>
#include <functional>
#include <string>
struct Point {
  int x;
  int y;
  float b;
  std::string name;
  std::filesystem::path path;
};

constexpr void for_each_member(Point &data, auto &&callback) {
  callback(data.x, "int", "x", 0);
  callback(data.y, "int", "y", 1);
  callback(data.b, "float", "b", 2);
  callback(data.name, "std::string", "name", 3);
  callback(data.path, "std::filesystem::path", "path", 4);
}

#include <optional>
#include "reflection.hpp"
#include <iostream>
int main() {
  using namespace creflec;
  Point point;
  std::string member_names[get_member_size(point)];
  std::cout << "Point has " << get_member_size(point) << " members.\n";

  set_as(point, "y", 42);
  set_as(point, "name", "go");
  set_as(point, "path", "/root");
  auto v = get_as<int>(point, "y");
  std::cout << "y: " << get_as<int>(point, "y").value() << "\n\n";

  for_each_member(point, [](auto v, auto t, auto n, auto idx) {
    std::cout << t << " " << n << "[" << v << "] " << idx << "\n";
  });
}