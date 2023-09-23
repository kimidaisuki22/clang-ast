#pragma once
#include <optional>
#include <string_view>
namespace creflec {

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

constexpr int get_member_size(const auto &target) {
  int size{};
  for_each_member(target, [&size](auto, auto, auto, auto) { size++; });
  return size;
}
} // namespace creflec