cmake_minimum_required(VERSION 3.18)
project(hello-llvm)

find_package(LLVM 11.0.0 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LT_LLVM_INSTALL_DIR}")

message("LLVM STATUS:
  Definitions ${LLVM_DEFINITIONS}
  Includes    ${LLVM_INCLUDE_DIRS}
  Libraries   ${LLVM_LIBRARY_DIRS}
  Targets     ${LLVM_TARGETS_TO_BUILD}"
)

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

add_executable(hello ${SOURCE_FILES})

llvm_map_components_to_libnames(llvm_libs 
  core
  engine
)
target_link_libraries(hello ${llvm_libs})
