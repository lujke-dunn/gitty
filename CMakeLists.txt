cmake_minimum_required(VERSION 3.14)
project(a_git_clone)

set(CMAKE_CXX_STANDARD 20)

# Find dependencies
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)

# Core library
add_library(gitty_core STATIC
        src/commands/command_factory.cpp
        src/commands/init_command.cpp
        src/commands/commit_command.cpp
        src/commands/add_command.cpp
        src/commands/remote_command.cpp
        src/commands/push_command.cpp
        src/commands/branch_command.cpp
        src/commands/command_option.cpp
        src/core/command.cpp
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
        src/core/refs.cpp
        src/commands/checkout_command.cpp
        src/commands/log_command.cpp
        src/commands/status_command.cpp
        src/commands/diff_command.cpp
)
target_include_directories(gitty_core PUBLIC include)
target_link_libraries(gitty_core
        OpenSSL::SSL
        OpenSSL::Crypto
        ZLIB::ZLIB
        CURL::libcurl
)

# Main executable
add_executable(a_git_clone src/main.cpp)
target_link_libraries(a_git_clone gitty_core)

# Testing
enable_testing()

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Test executable
add_executable(gitty_tests
        tests/helpers/test_repo.cpp
        tests/integration/init_test.cpp
        tests/integration/add_test.cpp
)
target_link_libraries(gitty_tests
        gitty_core
        GTest::gtest_main
)
target_include_directories(gitty_tests PRIVATE include tests/helpers)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(gitty_tests)