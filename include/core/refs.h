#ifndef A_GIT_CLONE_REFS_H
#define A_GIT_CLONE_REFS_H
#include <filesystem>

namespace fs = std::filesystem;

class Refs {
  private:
    fs::path gitPath;

    std::string readFile(const fs::path& path);
    void writeFile(const fs::path& path, const std::string& content);
    std::string trimWhitespace(const std::string& str);
  public:
    Refs(const fs::path& gitPath);

    std::string getCurrentBranch();
    std::string getHeadCommit();
    bool isDetachedHead();
    bool branchExists(const std::string& branchName);
    std::string getBranchCommit(const std::string& branchName);

    void updateHead(const std::string& branchName);
    void updateHeadDetached(const std::string& branchName);
    void createBranch(const std::string& name, const std::string& commitSha);
    void deleteBranch(const std::string& branchName);
};


#endif