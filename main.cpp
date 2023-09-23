#include <clang-c/CXString.h>
#include <clang-c/Index.h>
#include <iostream>
#include <string>
#include <string_view>
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
    int i{};
    iter = fmt::format_to(
        iter, "constexpr void for_each_member({} &data,auto&& callback){{\n", p.name);
    for (auto &v : p.members) {
      iter = fmt::format_to(iter, "\tcallback(data.{},\"{}\", \"{}\", {});\n",
                            v.name, v.type, v.name, i);
      i++;
    }
    iter = fmt::format_to(iter, "}}\n");
    return iter;
  }
};
#include <unordered_map>
std::unordered_map<std::string, Class_info> classes;

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

        if ((checked) && in_unit) {
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

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
}