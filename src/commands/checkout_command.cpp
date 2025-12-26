#include "../../include/commands/checkout_command.h"
#include "../../include/commands/command_option.h"
#include "../../include/core/object_util.h"
#include "../../include/core/workspace.h"
#include "../../include/core/index.h"
#include "../../include/core/refs.h"
#include <iostream>

CheckoutCommand::CheckoutCommand() : createBranch(false) {
    addOption(std::make_unique<BoolOption>(
        "-b", "", &createBranch, "Create a new branch"
    ));
}

bool CheckoutCommand::isValidCommit(const std::string &sha) {
    if (sha.length() != 40) return false;

    fs::path objPath = gitPath / "objects" / sha.substr(0, 2) / sha.substr(2);
    return fs::exists(objPath);
}

std::string CheckoutCommand::getCommitTree(const std::string &commitSha) {
    std::string commitData = ObjectUtils::readObject(gitPath / "objects" , commitSha);
    auto [type, content] = ObjectUtils::parseObject(commitData);

    if (type != "commit") {
        throw std::runtime_error("Not a commit: " + commitSha);
    }

    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.find("tree ") == 0) {
            return line.substr(5);
        }
    }

    throw std::runtime_error("Commit has no tree.");
}

std::map<std::string, Entry> CheckoutCommand::readTreeRecursive(const std::string &treeSha, const std::string &prefix) {
    std::map<std::string, Entry> entries;
    std::string treeData = ObjectUtils::readObject(gitPath / "objects", treeSha);
    auto [type, content] = ObjectUtils::parseObject(treeData);

    if (type != "tree") {
        throw std::runtime_error("Not a tree " + treeSha);
    }

    // time to parse tree entries
    size_t pos = 0;
    while (pos < content.length()) {
        size_t spacePos = content.find(' ', pos);
        if (spacePos == std::string::npos) break;

        std::string mode = content.substr(pos, spacePos - pos);
        pos = spacePos + 1;

        size_t nullPos = content.find('\0', pos);
        if (nullPos == std::string::npos) break;

        std::string name = content.substr(pos, nullPos - pos);
        pos = nullPos + 1;

        if (pos + 20 > content.length()) break;

        std::string sha1Binary = content.substr(pos, 20);
        pos += 20;

        std::stringstream ss;
        for (unsigned char c : sha1Binary) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        }
        std::string oid = ss.str();

        std::string fullPath = prefix.empty() ? name : prefix + "/" + name;

        if (mode == "40000") {
            // a subtree!
            auto subEntries = readTreeRecursive(oid, fullPath);
            entries.insert(subEntries.begin(), subEntries.end());
        } else {
            // a blob!
            int modeInt = std::stoi(mode, nullptr, 8);
            entries.emplace(fullPath, Entry(name, oid, modeInt));
        }
    }

    return entries;
}

void CheckoutCommand::updateWorkspace(const std::string& commitSha) {
    std::string treeSha = getCommitTree(commitSha);
    auto entries = readTreeRecursive(treeSha);

    Workspace workspace(rootPath);

    for (const auto& [path, entry] : entries) {
        std::string blobData = ObjectUtils::readObject(gitPath / "objects", entry.getOid());
        auto [type, content] = ObjectUtils::parseObject(blobData);

        if (type != "blob") continue;

        fs::path filePath = rootPath / path;
        fs::create_directories(filePath.parent_path());

        std::ofstream file(filePath, std::ios::binary);
        file.write(content.c_str(), content.length());
    }
}

void CheckoutCommand::updateIndex(const std::string& commitSha) {
    std::string treeSha = getCommitTree(commitSha);
    auto entries = readTreeRecursive(treeSha);

    Index index(gitPath);
    index.clear();

    for (const auto& [path, entry] : entries) {
        index.add(path, entry.getOid(), entry.getMode());
    }
}

void CheckoutCommand::checkoutBranch(const std::string& branch) {
    Refs refs(gitPath);

    if (!refs.branchExists(branch)) {
        std::cerr << "error: pathspec '" << branch << "' did not match any file(s) known to git" << std::endl;
        throw std::runtime_error("Branch not found");
    }

    std::string commitSha = refs.getBranchCommit(branch);

    refs.updateHead(branch);
    updateIndex(commitSha);
    updateWorkspace(commitSha);

    std::cout << "Switched to branch '" << branch  << "'" << std::endl;
}

void CheckoutCommand::checkoutCommit(const std::string& commitSha) {
    if (!isValidCommit(commitSha)) {
        std::cerr << "error: pathspec '" << commitSha << "' did not match any file(s) known to git" << std::endl;
        throw std::runtime_error("Invalid commit");
    }

    Refs refs(gitPath);
    refs.updateHeadDetached(commitSha);

    updateIndex(commitSha);
    updateWorkspace(commitSha);

    std::cout << "Note: switching to '" << commitSha << "'." << std::endl;
    std::cout << "You are in detached HEAD' state." << std::endl;
}

void CheckoutCommand::createAndCheckout(const std::string& branch) {
    Refs refs(gitPath);

    std::string headCommit = refs.getHeadCommit();
    if (headCommit.empty()) {
        throw std::runtime_error("cannot create branch - no commits yet");
    }

    refs.createBranch(branch, headCommit);
    refs.updateHead(branch);

    std::cout << "Switched to a new branch '" << branch << "'" << std::endl;
}

int CheckoutCommand::execute() {
    if (positionalArgs.empty()) {
        std::cerr << "error: please specify a branch or commit" << std::endl;
        return 1;
    }

    std::string target = positionalArgs[0];

    try {
        if (createBranch) {
            createAndCheckout(target);
        } else {
            if (Refs refs(gitPath); refs.branchExists(target)) {
                checkoutBranch(target);
            } else if (isValidCommit(target)) {
                checkoutCommit(target);
            } else {
                std::cerr << "error: pathspec '" << target << "' did not match any file(s) known to gitty" << std::endl;
                return 1;
            }
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }


}
