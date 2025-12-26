cmake_minimum_required(VERSION 4.0)
project(a_git_clone)

set(CMAKE_CXX_STANDARD 20)

add_executable(a_git_clone 
  src/main.cpp
  src/commands/command_factory.cpp
  src/commands/init_command.cpp
  src/commands/commit_command.cpp
  src/commands/add_command.cpp
  src/commands/remote_command.cpp
  src/commands/push_command.cpp
  src/commands/branch_command.cpp
  src/commands/command_option.cpp
  src/core/command.cpp
  src/core/git_object.cpp
  src/core/workspace.cpp
  src/core/blob.cpp 
  src/core/database.cpp
  src/core/config.cpp
  src/core/entry.cpp
  src/core/tree.cpp
  src/core/tree_builder.cpp
  src/core/remote.cpp
  src/core/author.cpp
  src/core/commit.cpp
  src/core/index.cpp
  src/core/object_util.cpp
  src/core/object_walker.cpp
  src/core/pack_file.cpp
  src/core/git_protocol.cpp
        include/core/refs.h
        src/core/refs.cpp
        include/commands/checkout_command.h
        src/commands/checkout_command.cpp
)

target_include_directories(a_git_clone PRIVATE include)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)
target_link_libraries(a_git_clone OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB CURL::libcurl)
