#include <clang-c/CXString.h>
#include <clang-c/Index.h>
#include <iostream>
#include <string>
#include <string_view>
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
        CXCursorKind accept_kind[] = {
            CXCursorKind::CXCursor_FieldDecl, CXCursorKind::CXCursor_ClassDecl,
            CXCursorKind::CXCursor_Namespace, CXCursorKind::CXCursor_EnumDecl,
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
        auto name = to_string(clang_getCursorSpelling(c));

        auto parent_name = to_string(clang_getCursorSpelling(parent));
        auto is_namespace = [&] { return kind == CXCursor_Namespace; }();

        if ((checked) && in_unit) {
          cout << "Cursor '" << clang_getCursorSpelling(c) << "' of kind '"
               << clang_getCursorKindSpelling(clang_getCursorKind(c))
               << "' name: " << clang_getCursorDisplayName(c)
               << " spelling: " << clang_getCursorSpelling(c) << " kind: "
               << clang_getCursorKindSpelling(clang_getCursorKind(c))
               << " parent: " << parent_name;
               if(kind ==  CXCursor_EnumConstantDecl){
                std::cout <<  " value: " << clang_getEnumConstantDeclValue(c);
               }
          std::cout << " type: "
                    << clang_getTypeSpelling(clang_getCursorType(c)) << "\n";
        }

        return CXChildVisit_Recurse;
      },
      nullptr);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
}
