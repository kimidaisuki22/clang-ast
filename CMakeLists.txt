cmake_minimum_required(VERSION 3.0.0)
project(ast VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(ast main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
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
