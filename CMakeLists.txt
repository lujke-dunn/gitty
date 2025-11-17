cmake_minimum_required(VERSION 4.0)
project(a_git_clone)

set(CMAKE_CXX_STANDARD 20)

add_executable(a_git_clone 
  src/main.cpp
  src/commands/command_factory.cpp
  src/commands/init_command.cpp
  src/commands/commit_command.cpp
  src/core/git_object.cpp
  src/core/workspace.cpp
  src/core/blob.cpp 
  src/core/database.cpp
)

target_include_directories(a_git_clone PRIVATE include)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
target_link_libraries(a_git_clone OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB)
