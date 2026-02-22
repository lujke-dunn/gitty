#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <array>
#include "test_repo.h"
#include "commands/commit_command.h"
#include "commands/add_command.h"
#include "core/index.h"

namespace fs = std::filesystem;

class CommitCommandTest : public ::testing::Test {
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

    static int runCommitWithAuthor(const std::string& message, const std::string& authorStr) {
        std::vector<std::string> args = {"commit", "-m", message, "--author", authorStr};
        std::vector<char*> argv;
        for (auto& a : args) argv.push_back(a.data());

        CommitCommand cmd;
        return cmd.run(static_cast<int>(argv.size()), argv.data());
    }

    static int runCommit(const std::string& message) {
        return runCommitWithAuthor(message, "Test User <test@example.com>");
    }

    // Returns the OID stored in refs/heads/master, or "" if it doesn't exist yet.
    std::string readMasterRef() const {
        fs::path masterRef = gitPath / "refs" / "heads" / "master";
        if (!fs::exists(masterRef)) return "";
        std::ifstream f(masterRef);
        std::string oid;
        std::getline(f, oid);
        oid.erase(oid.find_last_not_of(" \t\r\n") + 1);
        return oid;
    }

    bool objectExists(const std::string& oid) const {
        if (oid.size() < 3) return false;
        fs::path objPath = gitPath / "objects" / oid.substr(0, 2) / oid.substr(2);
        return fs::exists(objPath);
    }

    std::string runGit(const std::string& cmd) const {
        std::string full = "cd " + repo->getPath() + " && git " + cmd + " 2>&1";
        std::array<char, 8196> buffer;
        std::string result;
        FILE* pipe = popen(full.c_str(), "r");
        if (!pipe) return "";
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) result += buffer.data();
        pclose(pipe);
        return result;
    }
};


/**
 * A successful commit returns exit code 0.
 */
TEST_F(CommitCommandTest, SuccessfulCommitReturnsZero) {
    repo->writeFile("hello.txt", "hello world");
    runAdd({"hello.txt"});
    EXPECT_EQ(runCommit("initial commit"), 0);
}

/**
 * Attempting to commit with no staged files returns a non-zero exit code.
 */
TEST_F(CommitCommandTest, CommitWithEmptyIndexReturnsNonZero) {
    EXPECT_NE(runCommit("initial commit"), 0);
}

/**
 * After a successful commit, refs/heads/master contains a valid 40-char hex OID.
 */
TEST_F(CommitCommandTest, CommitWritesMasterRef) {
    repo->writeFile("file.txt", "data");
    runAdd({"file.txt"});
    runCommit("initial commit");

    const std::string oid = readMasterRef();
    ASSERT_EQ(oid.length(), 40);
    EXPECT_TRUE(std::all_of(oid.begin(), oid.end(), ::isxdigit));
}

/**
 * The commit object written to refs/heads/master exists on disk in the object database.
 */
TEST_F(CommitCommandTest, CommitCreatesObjectOnDisk) {
    repo->writeFile("file.txt", "content");
    runAdd({"file.txt"});
    runCommit("initial commit");

    EXPECT_TRUE(objectExists(readMasterRef()));
}

/**
 * The first commit in a repository prints "(root-commit)" to stdout.
 */
TEST_F(CommitCommandTest, RootCommitOutputContainsRootCommitTag) {
    repo->writeFile("hello.txt", "hello");
    runAdd({"hello.txt"});

    testing::internal::CaptureStdout();
    runCommit("initial commit");
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("(root-commit)"), std::string::npos);
}

/**
 * Commits after the first do not print "(root-commit)".
 */
TEST_F(CommitCommandTest, NonRootCommitDoesNotHaveRootCommitTag) {
    repo->writeFile("first.txt", "first");
    runAdd({"first.txt"});
    runCommit("first commit");

    repo->writeFile("second.txt", "second");
    runAdd({"second.txt"});

    testing::internal::CaptureStdout();
    runCommit("second commit");
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output.find("(root-commit)"), std::string::npos);
}

/**
 * The commit message appears in the output line.
 */
TEST_F(CommitCommandTest, CommitOutputContainsMessage) {
    repo->writeFile("file.txt", "data");
    runAdd({"file.txt"});

    testing::internal::CaptureStdout();
    runCommit("add initial file");
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("add initial file"), std::string::npos);
}

/**
 * A commit message longer than 50 characters is truncated with "..." in the output.
 */
TEST_F(CommitCommandTest, LongCommitMessageIsTruncatedInOutput) {
    repo->writeFile("file.txt", "data");
    runAdd({"file.txt"});

    const std::string longMsg = "this is a very long commit message that easily exceeds fifty characters";

    testing::internal::CaptureStdout();
    runCommit(longMsg);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("..."), std::string::npos);
}

/**
 * A multiline commit message uses only the first line in the output summary.
 */
TEST_F(CommitCommandTest, MultilineMessageUsesFirstLineInOutput) {
    repo->writeFile("file.txt", "data");
    runAdd({"file.txt"});

    testing::internal::CaptureStdout();
    runCommit("first line\nsecond line\nthird line");
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("first line"), std::string::npos);
    EXPECT_EQ(output.find("second line"), std::string::npos);
}

/**
 * Each successive commit stores a different OID in refs/heads/master.
 */
TEST_F(CommitCommandTest, MasterRefChangesWithEachCommit) {
    repo->writeFile("first.txt", "first");
    runAdd({"first.txt"});
    runCommit("first commit");
    const std::string firstOid = readMasterRef();

    repo->writeFile("second.txt", "second");
    runAdd({"second.txt"});
    runCommit("second commit");
    const std::string secondOid = readMasterRef();

    EXPECT_NE(firstOid, secondOid);
    EXPECT_TRUE(objectExists(firstOid));
    EXPECT_TRUE(objectExists(secondOid));
}

/**
 * Committing multiple staged files in one commit succeeds.
 */
TEST_F(CommitCommandTest, CommitWithMultipleFiles) {
    repo->writeFile("a.txt", "aaa");
    repo->writeFile("b.txt", "bbb");
    repo->writeFile("c.txt", "ccc");
    runAdd({"a.txt", "b.txt", "c.txt"});

    EXPECT_EQ(runCommit("add three files"), 0);
    EXPECT_TRUE(objectExists(readMasterRef()));
}

/**
 * Files in subdirectories are committed correctly.
 */
TEST_F(CommitCommandTest, CommitWithSubdirectoryFiles) {
    repo->writeFile("src/main.cpp", "int main() {}");
    repo->writeFile("src/util.cpp", "void util() {}");
    runAdd({"src/main.cpp", "src/util.cpp"});

    EXPECT_EQ(runCommit("add source files"), 0);
    EXPECT_TRUE(objectExists(readMasterRef()));
}

/**
 * Real git identifies our generated commit objects as type "commit".
 */
TEST_F(CommitCommandTest, GitCanReadCommitType) {
    repo->writeFile("readme.txt", "Hello World");
    runAdd({"readme.txt"});
    runCommit("initial commit");

    const std::string oid = readMasterRef();
    std::string output    = runGit("cat-file -t " + oid);
    output.erase(output.find_last_not_of(" \t\r\n") + 1);

    EXPECT_EQ(output, "commit");
}

/**
 * Real git can extract the original commit message from our generated commit object.
 */
TEST_F(CommitCommandTest, GitCanReadCommitMessage) {
    repo->writeFile("file.txt", "content");
    runAdd({"file.txt"});
    runCommit("my important message");

    const std::string output = runGit("cat-file -p " + readMasterRef());
    EXPECT_NE(output.find("my important message"), std::string::npos);
}

/**
 * The second commit stores the first commit's OID as its parent field.
 */
TEST_F(CommitCommandTest, SecondCommitHasFirstCommitAsParent) {
    repo->writeFile("first.txt", "first");
    runAdd({"first.txt"});
    runCommit("first commit");
    const std::string firstOid = readMasterRef();

    repo->writeFile("second.txt", "second");
    runAdd({"second.txt"});
    runCommit("second commit");
    const std::string secondOid = readMasterRef();

    const std::string output = runGit("cat-file -p " + secondOid);
    EXPECT_NE(output.find("parent " + firstOid), std::string::npos);
}

/**
 * The root commit has no parent field in its object content.
 */
TEST_F(CommitCommandTest, RootCommitHasNoParentField) {
    repo->writeFile("file.txt", "content");
    runAdd({"file.txt"});
    runCommit("initial commit");

    const std::string output = runGit("cat-file -p " + readMasterRef());
    EXPECT_EQ(output.find("parent"), std::string::npos);
}
