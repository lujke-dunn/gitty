#include <gtest/gtest.h>
#include "../helpers/test_repo.h"
#include "../../a-git-clone/include/core/workspace.h"
#include "../../a-git-clone/include/commands/init_command.h"
#include <filesystem>

namespace fs = std::filesystem;


/**
 * A basic test of the git init command should create a .git folder on successful execution
 */
TEST(InitTest, CreatesGitDirectory) {
    TestRepo repo;

    // create an init command object and run its execute command
    InitCommand cmd(repo.getPath());
    int result = cmd.execute();

    // expect that the command throws no errors
    EXPECT_EQ(result, 0);

    // expect that a ".git" directory has been made and is of directory type
    fs::path gitDir = fs::path(repo.getPath()) / ".git";
    EXPECT_TRUE(fs::exists(gitDir));
    EXPECT_TRUE(fs::is_directory(gitDir));
}

/**
 * Ensure object directory has been created
 */
TEST(InitTest, CreatesObjectsDirectory) {
    TestRepo repo;

    InitCommand cmd(repo.getPath());
    int result = cmd.execute();

    EXPECT_EQ(result, 0);

    fs::path objectsDir = fs::path(repo.getPath()) / ".git" / "objects";
    EXPECT_TRUE(fs::exists(objectsDir));
    EXPECT_TRUE(fs::is_directory(objectsDir));
}

/**
 * Ensure refs and remote head directory has been made
 */
TEST(InitTest, CreatesRefsHeadsDirectory) {
    TestRepo repo;

    InitCommand cmd(repo.getPath());
    int result = cmd.execute();

    EXPECT_EQ(result, 0);

    fs::path refsHeads = fs::path(repo.getPath()) / ".git" / "refs" / "heads";
    EXPECT_TRUE(fs::exists(refsHeads));
    EXPECT_TRUE(fs::is_directory(refsHeads));
}

/**
 * Ensure a HEAD file has been created
 */
TEST(InitTest, CreatesHeadFile) {
    TestRepo repo;

    InitCommand cmd(repo.getPath());
    int result = cmd.execute();

    EXPECT_EQ(result, 0);

    fs::path headFile = fs::path(repo.getPath()) / ".git" / "HEAD";
    EXPECT_TRUE(fs::exists(headFile));

    std::ifstream file(headFile);
    std::string content;
    std::getline(file, content);
    EXPECT_EQ(content, "ref: refs/heads/master");
}

/**
 * Ensure there is a config file which contains the repository version format
 */
TEST(InitTest, ConfigContainsRepositoryVersion) {
    TestRepo repo;

    InitCommand cmd(repo.getPath());
    int result = cmd.execute();

    EXPECT_EQ(result, 0);

    fs::path configFile = fs::path(repo.getPath()) / ".git" / "config";
    std::ifstream file(configFile);
    std::string   content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    EXPECT_TRUE(content.find("repositoryformatversion") != std::string::npos);
}

/**
 * Ensure gitty doesn't stomp over already populated file on repository init
 */
TEST(InitTest, CanInitOnNonEmptyDirectory) {
    TestRepo repo;

    repo.writeFile("already_here.txt", "I was here first");

    InitCommand cmd(repo.getPath());
    int result = cmd.execute();

    EXPECT_EQ(result, 0);

    std::string content = repo.readFile("already_here.txt");
    EXPECT_EQ(content, "I was here first");

    fs::path gitDir = fs::path(repo.getPath()) / ".git";
    EXPECT_TRUE(fs::exists(gitDir));
}

/**
 * Ensure git can read gitty repo format to ensure consistency with git
 */

TEST(InitTest, GitCanReadGittyRepo) {
    TestRepo repo;

    InitCommand cmd(repo.getPath());
    cmd.execute();

    std::string gitCmd = "cd " + repo.getPath() + " && git status 2>&1";
    FILE* pipe = popen(gitCmd.c_str(), "r");
    ASSERT_NE(pipe, nullptr);

    char buffer[128];
    std::string result;

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    int exitCode = pclose(pipe);

    EXPECT_EQ(exitCode, 0);
    EXPECT_TRUE(result.find("On branch master") != std::string::npos
        || result.find("No commit yet") != std::string::npos);
}

/**
 * Ensure initialization message is correct
 */
TEST(InitTest, PrintsSuccessMessage) {
    TestRepo repo;

    testing::internal::CaptureStdout();

    InitCommand cmd(repo.getPath());
    cmd.execute();

    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("Initialized empty git repository") != std::string::npos);
    EXPECT_TRUE(output.find(".git") != std::string::npos);
}

/**
 * Ensure gitty init works when a repo is already
 * initialized and does not stomp over preconfigured items within the repository
 */
TEST(InitTest, CanReinitializeExistingRepo) {
    TestRepo repo;

    InitCommand cmd1(repo.getPath());
    cmd1.execute();

    repo.writeFile("test.txt", "my passwords");

    InitCommand cmd2(repo.getPath());
    int result = cmd2.execute();

    EXPECT_EQ(result, 0);

    // .git should still exist (hopefully)
    fs::path gitDir = fs::path(repo.getPath()) / ".git";
    EXPECT_TRUE(fs::exists(gitDir));

    // test.txt with my very important passwords should be left untouched
    std::string content = repo.readFile("test.txt");
    EXPECT_EQ(content, "my passwords");

    // folder structure should still exist
    EXPECT_TRUE(fs::exists(gitDir / "objects"));
    EXPECT_TRUE(fs::exists(gitDir / "refs"));
    EXPECT_TRUE(fs::exists(gitDir / "HEAD"));

}

/**
 * Ensure gitty init works when a repo is already
 * initialized and does not stomp over preconfigured items within the repository
 */
TEST(InitTest, DoesNotDestroyExistingGitDirectory) {
    TestRepo repo;

    InitCommand cmd1(repo.getPath());
    cmd1.execute();

    // create a custom marker file that technically shouldn't be there but...
    fs::path customFile = fs::path(repo.getPath()) / ".git" / "marker_file.txt";
    std::ofstream marker(customFile);
    marker << "I will survive! yeah yeah yeah";
    marker.close();

    // reinitialize
    InitCommand cmd2(repo.getPath());
    cmd2.execute();

    // marker file should still exist
    EXPECT_TRUE(fs::exists(customFile));

    std::ifstream file(customFile);
    std::string   content;
    std::getline(file, content);

    EXPECT_EQ(content, "I will survive! yeah yeah yeah");
}


