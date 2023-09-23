#include "reflection.hpp"
#include <filesystem>
#include <iostream>

class Box{};
enum class fox {
  red,
  blue,
  yellow,
  green = 24,
};

constexpr fox default_fox = fox::red;
struct vec3 {
  float x;
  float y;
  float z;
};
constexpr void for_each_member(vec3 &data, auto &&callback) {
  callback(data.x, "float", "x", 0);
  callback(data.y, "float", "y", 1);
  callback(data.z, "float", "z", 2);
}
struct User {
  std::string name;
  int id;
};
constexpr void for_each_member(User &data, auto &&callback) {
  callback(data.name, "std::string", "name", 0);
  callback(data.id, "int", "id", 1);
}
struct Point {
  int x;
  int y;
  Box b;
  std::string name;
  std::filesystem::path path;
};
constexpr void for_each_member(const Point &data, auto &&callback) {
  callback(data.x, "int", "x", 0);
  callback(data.y, "int", "y", 1);
  callback(data.b, "Constants::Box", "b", 2);
  callback(data.name, "std::string", "name", 3);
  callback(data.path, "std::filesystem::path", "path", 4);
}
int main() { std::cout << creflec::get_member_size(Point{}) << "\n"; }