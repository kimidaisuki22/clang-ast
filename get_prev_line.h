#pragma once
#include <algorithm>
#include <fstream>
#include <ios>
#include <string>

inline std::string get_prev_line(std::ifstream &file) {
  std::string str;
  int line_break_count{};
  while (file) {
    if (line_break_count == 2) {
      break;
    }
    if (file.tellg() == 0) {
      break;
    }
    auto ch = file.get();
    if (ch == '\n') {
      line_break_count++;
    } else {
      if (line_break_count > 0) {
        str.push_back(ch);
      }
    }
    file.seekg(-2, std::ios::cur);
  }
  std::reverse(str.begin(), str.end());
  return str;
}
inline std::string get_double_slash_comment(std::string str) {
  auto pos = str.find("//");
  if (pos == std::string::npos) {
    return {};
  }
  return str.substr(pos + 2);
}