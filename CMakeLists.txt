cmake_minimum_required(VERSION 3.25.0)
project(ast VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
include(CTest)
enable_testing()

add_executable(ast main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(GNUInstallDirs)
install(TARGETS ast)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER kiki)
include(CPack)

  find_package(LLVM CONFIG REQUIRED)

  list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
  include(HandleLLVMOptions)
  add_definitions(${LLVM_DEFINITIONS})

  target_include_directories(ast PRIVATE ${LLVM_INCLUDE_DIRS})

  # Find the libraries that correspond to the LLVM components that we wish to use
  llvm_map_components_to_libnames(llvm_libs Support Core IRReader  )

  find_package(Clang CONFIG REQUIRED )
  target_include_directories(ast PRIVATE ${CLANG_INCLUDE_DIRS})
  target_link_libraries(ast PRIVATE clangParse clangAST clangBasic libclang)

  # Link against LLVM libraries
  target_link_libraries(ast PRIVATE ${llvm_libs}  ${CLANG_LIBS})

  find_package(fmt CONFIG REQUIRED)
  find_package(nlohmann_json CONFIG REQUIRED)
  find_package(CLI11 CONFIG REQUIRED)
  target_link_libraries(ast PRIVATE fmt::fmt)
  add_executable(test_p test.cpp)
  target_link_libraries(test_p PRIVATE CLI11::CLI11 fmt::fmt  nlohmann_json::nlohmann_json)