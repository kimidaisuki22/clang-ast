#pragma once
#include <filesystem>
#include <string>
namespace Constants {

class Box;
enum class fox {
  red,
  blue,
  yellow,
  green = 24,
};

constexpr fox default_fox = fox::red;

class Point {
  int x;
  int y;
  Box b;
  std::string name;
  std::filesystem::path path;

public:
  int len() const;
};

auto lgm = [] { return 42; };

inline auto var = 12;
} // namespace Constants

namespace {
void closef();
}

class User {
  std::string name;
  int id;
};

struct vec3 {
  float x;
  float y, z;

  inline constexpr static int size = 3;

  float *data() { return &x; }
  const float *data() const { return &x; }
  float distance(vec3 v){
    return 0;
  }
  static vec3 one(){
    return {1,1,1};
  }
};