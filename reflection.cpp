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
bool set_as(auto &target, std::string_view target_name, const auto &value) {
  bool result = false;
  for_each_member(target, [&](auto &target_prop, auto, auto name, auto) {
    if constexpr (requires { target_prop = value; }) {
      if (name == target_name) {
        target_prop = value;
        result = true;
      }
    }
  });

  return result;
}
#include <optional>
template <typename T>
std::optional<T> get_as(auto &target, std::string_view target_name) {
  bool result = false;
  std::optional<T> result_value;
  for_each_member(target, [&](auto &target_prop, auto, auto name, auto) {
    if constexpr (requires {
                    requires std::is_same_v<
                        T, std::remove_reference_t<decltype(target_prop)>>;
                  }) {
      if (name == target_name) {
        result_value = target_prop;
        result = true;
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  });

  return result_value;
}

constexpr int get_member_size(auto &target) {
  int size;
  for_each_member(target, [&size](auto, auto, auto, auto) { size++; });
  return size;
}

#include <iostream>
int main() {
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