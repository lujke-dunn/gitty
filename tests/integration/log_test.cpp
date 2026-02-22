#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "test_repo.h"
#include "commands/log_command.h"
#include "commands/add_command.h"
#include "commands/commit_command.h"

namespace fs = std::filesystem;

class LogCommandTest : public ::testing::Test {
protected:
    std::unique_ptr<TestRepo> repo;
    std::string               savedCwd;
    fs::path                  gitPath;

    void SetUp() override {
        savedCwd = fs::current_path().string();
        repo     = std::make_unique<TestRepo>();
        gitPath  = fs::path(repo->getPath()) / ".git";

        fs::create_directories(gitPath / "objects");
        fs::create_directories(gitPath / "refs" / "heads");

        std::ofstream head(gitPath / "HEAD");
        head << "ref: refs/heads/master\n";

        fs::current_path(repo->getPath());
    }

    void TearDown() override {
        fs::current_path(savedCwd);
    }

    static int runAdd(std::vector<std::string> files) {
        std::vector<std::string> args = {"add"};
        args.insert(args.end(), files.begin(), files.end());

        std::vector<char*> argv;
        for (auto& a : args) argv.push_back(a.data());

        AddCommand cmd;
        return cmd.run(static_cast<int>(argv.size()), argv.data());
    }

    static int runCommit(const std::string& message) {
        std::vector<std::string> args = {"commit", "-m", message, "--author", "Test User <test@example.com>"};
        std::vector<char*> argv;
        for (auto& a : args) argv.push_back(a.data());

        CommitCommand cmd;
        return cmd.run(static_cast<int>(argv.size()), argv.data());
    }

    static int runLog() {
        std::vector<std::string> args = {"log"};
        std::vector<char*> argv;
        for (auto& a : args) argv.push_back(a.data());

        LogCommand cmd;
        return cmd.run(static_cast<int>(argv.size()), argv.data());
    }

    // Returns the OID stored in refs/heads/master, or "" if it doesn't exist.
    std::string readMasterRef() const {
        fs::path masterRef = gitPath / "refs" / "heads" / "master";
        if (!fs::exists(masterRef)) return "";
        std::ifstream f(masterRef);
        std::string oid;
        std::getline(f, oid);
        oid.erase(oid.find_last_not_of(" \t\r\n") + 1);
        return oid;
    }
};


/**
 * When there are no commits, the log command must return a non-zero exit code
 * because there is nothing to display and a fatal error is expected.
 */
TEST_F(LogCommandTest, LogReturnNonZeroOnEmptyRepo) {
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    int result = runLog();
    testing::internal::GetCapturedStdout();
    testing::internal::GetCapturedStderr();

    EXPECT_NE(result, 0);
}

/**
 * After at least one commit has been made, the log command must return 0
 * indicating success.
 */
TEST_F(LogCommandTest, LogReturnsZeroWithCommits) {
    repo->writeFile("file.txt", "content");
    runAdd({"file.txt"});
    runCommit("initial commit");

    testing::internal::CaptureStdout();
    int result = runLog();
    testing::internal::GetCapturedStdout();

    EXPECT_EQ(result, 0);
}

/**
 * The log output must contain the full 40-character SHA of the commit that
 * HEAD currently points to, prefixed with "commit ".
 */
TEST_F(LogCommandTest, LogPrintsCommitSha) {
    repo->writeFile("file.txt", "content");
    runAdd({"file.txt"});
    runCommit("initial commit");

    const std::string expectedOid = readMasterRef();
    ASSERT_EQ(expectedOid.length(), 40) << "Precondition: master ref must be a valid 40-char OID";

    testing::internal::CaptureStdout();
    runLog();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("commit " + expectedOid), std::string::npos);
}

/**
 * The "Author:" line in the log output must contain the name portion of the
 * author string passed via --author when the commit was created.
 */
TEST_F(LogCommandTest, LogPrintsAuthorName) {
    repo->writeFile("file.txt", "data");
    runAdd({"file.txt"});
    runCommit("initial commit");

    testing::internal::CaptureStdout();
    runLog();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Author: Test User"), std::string::npos);
}

/**
 * The commit message used when committing must appear in the log output,
 * indented under the date line.
 */
TEST_F(LogCommandTest, LogPrintsCommitMessage) {
    repo->writeFile("file.txt", "data");
    runAdd({"file.txt"});
    runCommit("my very specific commit message");

    testing::internal::CaptureStdout();
    runLog();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("my very specific commit message"), std::string::npos);
}

/**
 * When two commits exist, the log must show both of them — it walks the full
 * parent chain from HEAD back to the root.
 */
TEST_F(LogCommandTest, LogShowsAllCommits) {
    repo->writeFile("first.txt", "first");
    runAdd({"first.txt"});
    runCommit("first commit");

    repo->writeFile("second.txt", "second");
    runAdd({"second.txt"});
    runCommit("second commit");

    testing::internal::CaptureStdout();
    runLog();
    std::string output = testing::internal::GetCapturedStdout();

    // Both messages must appear in the output.
    EXPECT_NE(output.find("first commit"), std::string::npos);
    EXPECT_NE(output.find("second commit"), std::string::npos);
}

/**
 * Log must print commits in reverse-chronological order (most recent first).
 * After two commits, the second commit's SHA must appear earlier in the output
 * than the first commit's SHA.
 */
TEST_F(LogCommandTest, LogShowsMostRecentCommitFirst) {
    repo->writeFile("first.txt", "first");
    runAdd({"first.txt"});
    runCommit("first commit");
    const std::string firstOid = readMasterRef();

    repo->writeFile("second.txt", "second");
    runAdd({"second.txt"});
    runCommit("second commit");
    const std::string secondOid = readMasterRef();

    ASSERT_NE(firstOid, secondOid) << "Precondition: two commits must produce distinct OIDs";

    testing::internal::CaptureStdout();
    runLog();
    std::string output = testing::internal::GetCapturedStdout();

    const auto posFirst  = output.find(firstOid);
    const auto posSecond = output.find(secondOid);

    ASSERT_NE(posFirst,  std::string::npos) << "First commit OID not found in log output";
    ASSERT_NE(posSecond, std::string::npos) << "Second commit OID not found in log output";

    // The most recent (second) commit must appear before the older (first) commit.
    EXPECT_LT(posSecond, posFirst);
}
