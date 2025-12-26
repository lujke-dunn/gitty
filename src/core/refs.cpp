#include "../../include/core/refs.h"
#include <fstream>
#include <stdexcept>

Refs::Refs(const fs::path& gitPath) : gitPath(gitPath) {}

std::string Refs::readFile(const fs::path& path) {
    if (!fs::exists(path)) {
        return "";
    }
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::string content;
    std::getline(file, content);
    return trimWhitespace(content);
}

void Refs::writeFile(const fs::path& path, const std::string& content) {
    fs::create_directories(path.parent_path());
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to write to " + path.string());
    }
    file << content << std::endl;
}

std::string Refs::trimWhitespace(const std::string& str) {
    if (str.empty()) return str;

    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string Refs::getCurrentBranch() {
    fs::path headPath = gitPath / "HEAD";
    std::string headContent = readFile(headPath);

    if (headContent.find("ref: refs/heads/") == 0) {
        return headContent.substr(16);  // Skip "ref: refs/heads/"
    }

    return "";  // Detached HEAD
}

std::string Refs::getHeadCommit() {
    fs::path headPath = gitPath / "HEAD";
    std::string headContent = readFile(headPath);

    if (headContent.find("ref: ") == 0) {
        // HEAD points to a branch
        std::string refPath = headContent.substr(5);
        fs::path refFilePath = gitPath / refPath;
        return readFile(refFilePath);
    }

    // Detached HEAD - return the commit SHA directly
    return headContent;
}

bool Refs::isDetachedHead() {
    fs::path headPath = gitPath / "HEAD";
    std::string headContent = readFile(headPath);
    return headContent.find("ref: ") != 0;
}

bool Refs::branchExists(const std::string& branch) {
    fs::path branchPath = gitPath / "refs" / "heads" / branch;
    return fs::exists(branchPath);
}

std::string Refs::getBranchCommit(const std::string& branch) {
    fs::path branchPath = gitPath / "refs" / "heads" / branch;
    return readFile(branchPath);
}

void Refs::updateHead(const std::string& branchName) {
    fs::path headPath = gitPath / "HEAD";
    writeFile(headPath, "ref: refs/heads/" + branchName);
}

void Refs::updateHeadDetached(const std::string& commitSha) {
    fs::path headPath = gitPath / "HEAD";
    writeFile(headPath, commitSha);
}

void Refs::createBranch(const std::string& name, const std::string& commitSha) {
    if (branchExists(name)) {
        throw std::runtime_error("Branch '" + name + "' already exists");
    }

    fs::path branchPath = gitPath / "refs" / "heads" / name;
    writeFile(branchPath, commitSha);
}

void Refs::deleteBranch(const std::string& name) {
    if (!branchExists(name)) {
        throw std::runtime_error("Branch '" + name + "' not found");
    }

    fs::path branchPath = gitPath / "refs" / "heads" / name;
    fs::remove(branchPath);
}