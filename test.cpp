#include "reflection.hpp"
#include "reflection_ext.hpp"
#include <filesystem>
#include <iostream>

class Box {};
enum class fox {
  red,
  blue,
  yellow,
  green = 24,
};

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
  callback(data.name, "std::string", "name", 3);
  callback(data.path, "std::filesystem::path", "path", 4);
}
constexpr void for_each_member(Point &data, auto &&callback) {
  callback(data.x, "int", "x", 0);
  callback(data.y, "int", "y", 1);
  callback(data.name, "std::string", "name", 3);
  callback(data.path, "std::filesystem::path", "path", 4);
}
constexpr void for_each_member(Point &&data, auto &&callback) {
  callback(data.x, "int", "x", 0);
  callback(data.y, "int", "y", 1);
  callback(data.name, "std::string", "name", 3);
  callback(data.path, "std::filesystem::path", "path", 4);
}
#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
int main(int argc, char **argv) {
  CLI::App app{"App description"};
  Point point;
  creflec::bind_cli11(app,point);

  CLI11_PARSE(app, argc, argv);
  std::cout << creflec::get_member_size(Point{}) << "\n";
  Point p;
  const auto cp = p;
  std::cout << creflec::get_member_size(p) << "\n";
  std::cout << creflec::get_member_size(cp) << "\n";
  for_each_member(cp, creflec::print_call_back);
  nlohmann::json json;
  creflec::to_json(p, json);
  std::cout << json << "\n";
}