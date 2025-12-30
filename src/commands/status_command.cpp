#include "../../include/commands/status_command.h"
#include "../../include/core/refs.h"
#include "../../include/core/index.h"
#include "../../include/core/workspace.h"
#include "../../include/core/object_util.h"
#include <iostream>
#include <sstream>
#include <functional>
#include <sys/stat.h>

StatusCommand::StatusCommand() {}

std::map<std::string, std::string> StatusCommand::getHeadTree() {
    std::map<std::string, std::string> headFiles;

    Refs refs(gitPath);
    std::string headCommit = refs.getHeadCommit();
    if (headCommit.empty()) {
        return headFiles;
    }

    std::string commitData = ObjectUtils::readObject(gitPath / "objects", headCommit);
    auto [type, content]   = ObjectUtils::parseObject(commitData);
    if (type != "commit") return headFiles;

    std::istringstream stream(content);
    std::string line;
    std::string treeSha;

    while (std::getline(stream, line)) {
        if (line.find("tree ") == 0) {
            treeSha = line.substr(5);
            break;
        }
    }
    if (treeSha.empty()) return headFiles;

    std::function<void(const std::string&, const std::string&)> readTree = [&](const std::string& tSha, const std::string& prefix) {
        std::string treeData = ObjectUtils::readObject(gitPath / "objects", tSha);
        auto [tType, tContent] = ObjectUtils::parseObject(treeData);

        if (tType != "tree") return;

        size_t pos = 0;
        while (pos < tContent.length()) {
            size_t spacePos = tContent.find(' ', pos);
            if (spacePos == std::string::npos) break;

            std::string mode = tContent.substr(pos, spacePos - pos);
            pos = spacePos + 1;

            size_t nullPos = tContent.find('\0', pos);
            if (nullPos == std::string::npos) break;

            std::string name = tContent.substr(pos, nullPos - pos);
            pos = nullPos + 1;

            if (pos + 20 > tContent.length()) break;

            std::string sha1Binary = tContent.substr(pos, 20);
            pos += 20;

            std::stringstream ss;
            for (unsigned char c : sha1Binary) {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
            }
            std::string oid = ss.str();

            std::string fullPath = prefix.empty() ? name : prefix + "/" + name;

            if (mode == "40000") {
                readTree(oid, fullPath);
            } else {
                headFiles[fullPath] = oid;
            }
        }
    };

    readTree(treeSha, "");
    return headFiles;
}

std::set<std::string> StatusCommand::getWorkspaceFiles() {
    Workspace workspace(rootPath);
    auto files = workspace.listFiles();
    return std::set<std::string>(files.begin(), files.end());
}

bool StatusCommand::fileChanged(const IndexEntry &entry) {
    fs::path filePath = rootPath / entry.path;

    struct stat st;
    if (stat(filePath.c_str(), &st) != 0) {
        return true; // file deleted
    }

    // see if size of mtime has changed
    if (st.st_size != entry.size || st.st_mtime != entry.mtime_sec) {
        return true; // file changed in size or last edited time changed
    }

    return false;
}

int StatusCommand::execute() {
    try {
        Refs refs(gitPath);
        std::string currentBranch = refs.getCurrentBranch();

        if (currentBranch.empty()) {
            std::cout << "HEAD detached at " << refs.getHeadCommit().substr(0, 7) << std::endl;
        } else {
            std::cout << "On branch " << currentBranch << std::endl;
        }

        Index index(gitPath);
        auto indexEntries = index.getEntries();
        auto headFiles = getHeadTree();
        auto worksapceFiles = getWorkspaceFiles();

        std::vector<std::string> staged;
        for (const auto& [path, entry] : indexEntries) {
            auto it = headFiles.find(path);
            if (it == headFiles.end() || it->second != entry.oid) {
                staged.push_back(path);
            }
        }

        std::vector<std::string> modified;
        for (const auto& [path, entry] : indexEntries) {
            if (worksapceFiles.count(path) && fileChanged(entry)) {
                modified.push_back(path);
            }
        }

        std::vector<std::string> deleted;
        for (const auto& [path, entry] : indexEntries) {
            if (!worksapceFiles.count(path)) {
                deleted.push_back(path);
            }
        }

        std::vector<std::string> untracked;
        for (const auto& path : worksapceFiles) {
            if (indexEntries.find(path) == indexEntries.end()) {
                untracked.push_back(path);
            }
        }

        // now display the info to the user
        if (staged.empty() && modified.empty() && deleted.empty() && untracked.empty()) {
            std::cout << "nothing to commit, working tree clean" << std::endl;
            return 0;
        }

        if (!staged.empty()) {
            std::cout << "\nChanges to be commited:" << std::endl;
            for (const auto& path : staged) {
                if (headFiles.find(path) == headFiles.end()) {
                    std::cout << "\tnew file:  " << path << std::endl;
                } else {
                    std::cout << "\tmodified:  " << path << std::endl;
                }
            }
        }

        if (!modified.empty() || !deleted.empty()) {
            std::cout << "\nChanges not staged for commit:" << std::endl;
            for (const auto& path : modified) {
                std::cout << "\tmodified:   " << path << std::endl;
            }
            for (const auto& path : deleted) {
                std::cout << "\tdeleted:    " << path << std::endl;
            }
        }

        if (!untracked.empty()) {
            std::cout << "\nUntracked files:" << std::endl;
            for (const auto& path : untracked) {
                std::cout << "\t" << path << std::endl;
            }
        }

        std::cout << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }
}
