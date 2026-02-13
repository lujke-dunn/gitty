#include "test_repo.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>

namespace fs = std::filesystem;

static std::string generateTempDirName() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000000, 9999999);

    std::ostringstream oss;
    oss << "gitty_test_" << dis(gen);
    return oss.str();
}


TestRepo::TestRepo() {
    fs::path tempDir = fs::temp_directory_path() / generateTempDirName();
    fs::create_directories(tempDir);
    tempPath = tempDir.string();
}

TestRepo::~TestRepo() {
    try {
        if (!tempPath.empty() && fs::exists(tempPath)) {
            fs::remove_all(tempPath);
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

std::string TestRepo::getPath() const {
    return tempPath;
}

void TestRepo::writeFile(const std::string &path, const std::string &content) {
    fs::path fullPath = fs::path(tempPath) / path;

    fs::create_directories(fullPath.parent_path());

    std::ofstream file(fullPath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + fullPath.string());
    }

    file << content;
    file.close();
}

std::string TestRepo::readFile(const std::string &path) {
    fs::path fullPath = fs::path(tempPath) / path;

    std::ifstream file(fullPath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for reading: " + fullPath.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
