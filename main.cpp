#include <clang-c/CXString.h>
#include <clang-c/Index.h>
#include <cstdint>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
using namespace std;

ostream &operator<<(ostream &stream, const CXString &str) {
  stream << clang_getCString(str);
  clang_disposeString(str);
  return stream;
}
std::string to_string(const CXString &str) {
  std::string s = clang_getCString(str);
  clang_disposeString(str);
  return s;
}
bool pos_check(std::string_view str, int pos, char ch) {
  if (str.size() <= pos) {
    return false;
  }
  return str[pos] == ch;
}

std::string src = "header.hpp";
struct Member_info {
  std::string type;
  std::string name;
};
struct Class_info {
  std::vector<Member_info> members;
  std::string name;
};

struct Enums {
  std::string parent;
  std::string parent_type;

  std::string name;
  uint64_t value;
};
std::unordered_map<std::string, std::vector<Enums>> enums_values;

#include <fmt/format.h>

template <> struct fmt::formatter<Member_info> {
  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context &ctx)
      -> format_parse_context::iterator {
    // [ctx.begin(), ctx.end()) is a character range that contains a part of
    // the format string starting from the format specifications to be parsed,
    // e.g. in
    //
    //   fmt::format("{:f} - point of interest", point{1, 2});
    //
    // the range will contain "f} - point of interest". The formatter should
    // parse specifiers until '}' or the end of the range. In this example
    // the formatter should parse the 'f' specifier and return an iterator
    // pointing to '}'.

    // Please also note that this character range may be empty, in case of
    // the "{}" format string, so therefore you should check ctx.begin()
    // for equality with ctx.end().

    // Parse the presentation format and store it in the formatter:
    auto it = ctx.begin(), end = ctx.end();

    // Check if reached the end of the range:
    // if (it != end && *it != '}')
    //   ctx.on_error("invalid format");

    // Return an iterator past the end of the parsed range:
    return it;
  }

  // Formats the point p using the parsed format specification (presentation)
  // stored in this formatter.
  auto format(const Member_info &p, format_context &ctx) const
      -> format_context::iterator {
    // ctx.out() is an output iterator to write to.
    return fmt::format_to(ctx.out(), "{} {};", p.type, p.name);
  }
};
template <> struct fmt::formatter<Class_info> {
  constexpr auto parse(format_parse_context &ctx)
      -> format_parse_context::iterator {
    auto it = ctx.begin(), end = ctx.end();

    return it;
  }
  auto format(const Class_info &p, format_context &ctx) const
      -> format_context::iterator {
    // ctx.out() is an output iterator to write to.
    auto iter = fmt::format_to(ctx.out(), "struct {}{{\n", p.name);
    for (auto &v : p.members) {
      iter = fmt::format_to(iter, "\t{}\n", v);
    }
    iter = fmt::format_to(iter, "}};\n");

    {
      int i{};
      std::string body;
      for (auto &v : p.members) {
        body += fmt::format("\tcallback(data.{},\"{}\", \"{}\", {});\n", v.name,
                            v.type, v.name, i);
        i++;
      }
      auto add_functions = [&](std::string_view const_str,
                               std::string_view ref_str) {
        iter = fmt::format_to(
            iter,
            "constexpr void for_each_member({} {} {}data,auto&& callback){{\n",
            const_str, p.name, ref_str);
        iter = fmt::format_to(iter, "{}", body);
        iter = fmt::format_to(iter, "}}\n");
      };
      add_functions("const", "&");
      add_functions("", "&");
      add_functions("", "&&");
    }
    return iter;
  }
};
#include <unordered_map>
std::unordered_map<std::string, Class_info> classes;
struct Member_function_info {
  std::string name;
  std::string type;

  std::string parent;
};
std::unordered_map<std::string, std::vector<Member_function_info>> methods;

int main() {
  CXIndex index = clang_createIndex(1, 0);
  const char *argv[] = {"-std=c++20"};
  CXTranslationUnit unit =
      clang_parseTranslationUnit(index, src.c_str(), argv, 1, nullptr, 0,
                                 CXTranslationUnit_SkipFunctionBodies
                                 // | CXTranslationUnit_SingleFileParse
      );
  if (unit == nullptr) {
    cerr << "Unable to parse translation unit. Quitting." << endl;
    exit(-1);
  }

  CXCursor cursor = clang_getTranslationUnitCursor(unit);

  clang_visitChildren(
      cursor,
      [](CXCursor c, CXCursor parent, CXClientData client_data) {
        auto location = clang_getCursorLocation(c);
        bool in_unit = clang_Location_isFromMainFile(location);
        auto kind = clang_getCursorKind(c);
        CXCursorKind accept_kind[] = {CXCursorKind::CXCursor_FieldDecl,
                                      CXCursorKind::CXCursor_ClassDecl,
                                      CXCursorKind::CXCursor_Namespace,
                                      CXCursorKind::CXCursor_EnumDecl,
                                      CXCursorKind::CXCursor_EnumConstantDecl,
                                      CXCursorKind::CXCursor_FunctionDecl,
                                      CXCursorKind::CXCursor_CXXMethod};
        bool checked = [&] {
          for (auto k : accept_kind) {
            if (k == kind) {
              return true;
            }
          }
          return false;
        }();
        auto name = to_string(clang_getCursorSpelling(c)); // or spelling.
        auto display_name = to_string(clang_getCursorDisplayName(c));

        auto kind_name =
            to_string(clang_getCursorKindSpelling(clang_getCursorKind(c)));

        auto parent_name = to_string(clang_getCursorSpelling(parent));
        auto is_namespace = [&] { return kind == CXCursor_Namespace; }();

        auto type_name =
            to_string(clang_getTypeSpelling(clang_getCursorType(c)));
        const bool is_target = checked && in_unit;
        if (is_target && kind == CXCursor_EnumConstantDecl) {
          Enums e{};
          e.value = clang_getEnumConstantDeclValue(c);
          e.parent = parent_name;
          e.parent_type = type_name;
          e.name = display_name;
          enums_values[e.parent_type].push_back(e);
        }
        if (is_target && kind == CXCursor_CXXMethod) {
          Member_function_info info{};
          info.name = display_name;
          info.type = type_name;
          info.parent = parent_name;

          methods[info.parent].push_back(info);
        }
        if (is_target) {
          cout << "Cursor '" << name << "' of kind '" << kind_name
               << "' name: " << display_name << " parent: " << parent_name;
          if (kind == CXCursor_EnumConstantDecl) {
            std::cout << " value: " << clang_getEnumConstantDeclValue(c);
          }
          std::cout << " type: " << type_name << "\n";

          if (kind == CXCursor_FieldDecl) {
            Member_info member;
            member.name = name;
            member.type = type_name;
            classes[parent_name].members.push_back(member);
          }
        }

        return CXChildVisit_Recurse;
      },
      nullptr);

  for (auto [n, c] : classes) {
    c.name = n;
    fmt::print("{}", c);
  }
  for (auto [e, values] : enums_values) {
    fmt::println("Enum: {}", e);
    for (auto v : values) {
      fmt::println("\t{}: {}", v.name, v.value);
    }
  }
  for (auto [class_name, values] : methods) {
    fmt::println("class: {}", class_name);
    for (auto v : values) {
      fmt::println("\t{}: {}", v.name, v.type);
    }
  }
  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
}
