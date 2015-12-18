cmake_minimum_required(VERSION 2.8)

project("NES Recompiler")

find_package(Boost COMPONENTS iostreams REQUIRED)
find_package(LLVM REQUIRED CONFIG)

add_executable(recompile src/main.cpp
  src/machine_spec.cpp
  src/nes_machine_spec.cpp
  src/instruction.cpp
  src/flow.cpp
  src/codegen.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(recompile ${Boost_LIBRARIES})
target_link_libraries(recompile ${llvm_libs})
