#ifndef A_GIT_CLONE_REFS_H
#define A_GIT_CLONE_REFS_H
#include <filesystem>

namespace fs = std::filesystem;

/**
 * Handles reading and writing reference files in .git/refs/ and .git/HEAD.
 * References are stored as text files containing commit hashes.
 *
 * HEAD can be a symbolic ref: "ref: refs/heads/main" (pointing to a branch) or HEAD can be a detached commit sha (not on any branch).
 *
 * Branches are files at .git/refs/heads/<branch-name> containing commit SHAs.
 */
class Refs {
  private:
    fs::path gitPath;

    std::string readFile(const fs::path& path);
    void writeFile(const fs::path& path, const std::string& content);
    std::string trimWhitespace(const std::string& str);
  public:
    Refs(const fs::path& gitPath);

    /**
     * Gets the current branch name.
     *
     * @return Branch name (e.g., "main"), or empty string if detached HEAD
     */
    std::string getCurrentBranch();

    /**
     * Gets the commit SHA that HEAD points to.
     * Resolves symbolic refs (follows branch -> commit).
     *
     * @return a commit hash
     */
    std::string getHeadCommit();

    /**
     * Check if HEAD is detached (points directly to commit, not a branch).
     *
     * @return true if HEAD contains a commit SHA instead of "ref: ..."
     */
    bool isDetachedHead();

    /**
     * Checks if branch exists.
     *
     * @param branchName name of branch without refs/heads/ prefix
     * @return bool denoting whether branch exists lmao.
     */
    bool branchExists(const std::string& branchName);

    /**
     * Gets the commit SHA a branch points to.
     *
     * @param branchName name of branch
     * @return a commit SHA, or empty string if branch doesn't exist
     */
    std::string getBranchCommit(const std::string& branchName);

    /**
     * Creates a new branch pointing to a commit.
     *
     * @param branchName commit SHA for branch to point to
     */
    void updateHead(const std::string& branchName);

    /**
     * Updates HEAD to point directly to a commit (detached HEAD).
     * Sets HEAD content to the commit SHA.
     *
     * @param branchName to delete
     */
    void updateHeadDetached(const std::string& branchName);

    /**
     * Creates a new branch pointing to a commit.
     *
     * @param name of branch
     * @param commitSha for branch to point to
     */
    void createBranch(const std::string& name, const std::string& commitSha);

    /**
     * Deletes a branch.
     *
     * @param branchName of branch to delete
     */
    void deleteBranch(const std::string& branchName);
};


#endif