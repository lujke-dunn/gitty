#include <filesystem>
#include <openssl/sha.h>

#include "test_repo.h"
#include "commands/add_command.h"
#include "core/index.h"
#include "gtest/gtest.h"

namespace fs = std::filesystem;

class AddCommandTest : public ::testing::Test {
protected:
    std::unique_ptr<TestRepo> repo;
    std::string               savedCwd;

    void SetUp() override {
        savedCwd = fs::current_path().string();
        repo     = std::make_unique<TestRepo>();
        fs::create_directories(fs::path(repo->getPath()) / ".git" / "objects");
        fs::current_path(repo->getPath());
    }

    void TearDown() override {
        fs::current_path(savedCwd);
    }

    /**
     * A getter for the .git/objects path where objects for a given index are stored.
     * @param repo
     * @return the object path for the generated test repository
     */
    static fs::path getObjPath(const std::unique_ptr<TestRepo>::element_type& repo) {
        return fs::path(repo.getPath()) / ".git" / "objects";
    }

    /**
     * runs the add command, adding a file to the index
     *
     * @param args a list of arguments passed to the add command
     * @return executes the add command
     */
    static int runAdd(std::vector<std::string> args) {
        args.insert(args.begin(), "add");

        std::vector<char*> argv;
        for (auto& arg : args) argv.push_back(arg.data());

        AddCommand cmd;
        return cmd.run(static_cast<int>(argv.size()), argv.data());
    }

    /**
     * creates an Index for the repository and loads it into memory
     * @return Index for repository
     */
    Index loadIndex() const {
        return Index(fs::path(repo->getPath()) / ".git");
    }

    /**
     * Checks if a given oid is found within the index.
     *
     * @param oid the object identifier
     * @return whether the object exists in the index
     */
    bool objectExists(const std::string& oid) const {
        const fs::path objPath     = ".git/objects";
        const fs::path oidPath     = fs::path(oid.substr(0,2)) / oid.substr(2);
        fs::path fullObjPath = fs::path(repo->getPath()) / objPath / oidPath;

        return fs::exists(fullObjPath);
    }

    /**
     * calculates the amount of IndexEntry 's there are
     * in a given repositories index.
     *
     * @return the amount of objects within the index
     */
    int countObjects() const {
        int count = 0;
        const fs::path objDir = getObjPath(*repo);
        for (auto& entry : fs::recursive_directory_iterator(objDir)) {
            if (entry.is_regular_file()) count++;
        }
        return count;
    }

    /**
     * Compute the expected oid of a file.
     *
     * @param content the content of the file
     * @return a blob of the data that was once the file.
     */
    static std::string computeExpectedOid(const std::string& content) {
        std::string fullOid = "blob " + std::to_string(content.size()) + '\0' + content;
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char*>(fullOid.c_str()), fullOid.size(), hash);
        std::stringstream ss;
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return ss.str();
    }

    /**
     * Turn a regular file of type 100644 to an executable file of 100755
     * @param path the path to the file
     */
    void makeExecutable(const std::string& path) const {
        fs::path full = fs::path(repo->getPath()) / path;
        fs::permissions(full, fs::perms::owner_exec, fs::perm_options::add);
    }
};


/**
 * Add one single file to the index with one invocation of the add command
 * Example command: "gitty add who.txt"
 */
TEST_F(AddCommandTest, AddSingleFile) {
    repo->writeFile("who.txt", "who");
    ASSERT_EQ(runAdd({"who.txt"}), 0);

    auto index    = loadIndex();
    auto& entries = index.getEntries();

    const auto& entry = entries.at("who.txt");
    EXPECT_EQ(entry.oid.length(), 40);
    EXPECT_TRUE(objectExists(entry.oid));
}


/**
 * Ensure it adds a file to the index, with the correct corresponding oid
 */
TEST_F(AddCommandTest, AddSingleFileWithCorrectOid) {
    std::string content = "hello wordl";
    repo->writeFile("wordl.txt", content);
    runAdd({"wordl.txt"});

    auto index = loadIndex();
    auto& entries = index.getEntries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_TRUE(entries.count("wordl.txt"));

    const auto& entry = entries.at("wordl.txt");
    EXPECT_EQ(entry.oid.length(), 40);
    EXPECT_TRUE(objectExists(entry.oid));
}

/**
 * Add multiple files to index with one invocation of the add command
 * Example command: "gitty add who.txt what.txt why.txt, when.php"
 */
TEST_F(AddCommandTest, AddMultipleFilesOneCommand) {
    repo->writeFile("who.txt", "who");
    repo->writeFile("what.txt", "what");
    repo->writeFile("why.txt", "why");
    repo->writeFile("when.php", "when");
    ASSERT_EQ(runAdd({"who.txt", "what.txt", "why.txt", "when.php"}), 0);

    auto index = loadIndex();
    ASSERT_EQ(index.getEntries().size(), 4);
    EXPECT_TRUE(index.isStaged("who.txt"));
    EXPECT_TRUE(index.isStaged("what.txt"));
    EXPECT_TRUE(index.isStaged("why.txt"));
    EXPECT_TRUE(index.isStaged("when.php"));
}

/**
 * Add multiple files to index with multiple invocation's of the add command
 * Example command:
 *                  gitty add who.txt
 *                  gitty add what.txt
 *                  gitty add why.txt
 *                  gitty add when.php
 */
TEST_F(AddCommandTest, AddMultipleFilesWithMultipleCommandInvocations) {
    // why would i ever write a function for this can i be fucked hell no
    repo->writeFile("who.txt", "who");
    repo->writeFile("what.txt", "what");
    repo->writeFile("why.txt", "why");
    repo->writeFile("when.php", "when");

    runAdd({"who.txt"});
    runAdd({"what.txt"});
    runAdd({"why.txt"});
    runAdd({"when.php"});

    auto index = loadIndex();
    ASSERT_EQ(index.getEntries().size(), 4);
    EXPECT_TRUE(index.isStaged("who.txt"));
    EXPECT_TRUE(index.isStaged("what.txt"));
    EXPECT_TRUE(index.isStaged("why.txt"));
    EXPECT_TRUE(index.isStaged("when.php"));
}

/**
 * Add all files that are currently unstaged to the index
 * Example command:
 *                  gitty add .
 */
TEST_F(AddCommandTest, AddDotSuccesfullyExpandsToAllUnstagedFiles) {
    repo->writeFile("what.txt", "what");
    repo->writeFile("who.txt", "who");
    repo->writeFile("when.php", "when");

    ASSERT_EQ(runAdd({"."}), 0);

    auto index = loadIndex();
    ASSERT_EQ(index.getEntries().size(), 3);
    EXPECT_TRUE(index.isStaged("what.txt"));
    EXPECT_TRUE(index.isStaged("who.txt"));
    EXPECT_TRUE(index.isStaged("when.php"));
}

/**
 * When invoking gitty add . ; it does not include the .git directory
 */
TEST_F(AddCommandTest, AddDotExcludesGitDirectory) {
    repo->writeFile("what.txt", "what");
    runAdd({"."});

    auto index = loadIndex();
    for (const auto& [path, _] : index.getEntries()) {
        EXPECT_TRUE(path.find(".git") == std::string::npos)
        << "Found .git path in index: " << path;
    }
}

/**
 * When working in a empty workspace invoking gitty add .
 * ensure nothing is added to the index
 */
TEST_F(AddCommandTest, AddDotDoesNotAddAnythingForEmptyDirectory) {
    ASSERT_EQ(runAdd({"."}), 0);
    auto index = loadIndex();
    EXPECT_EQ(index.getEntries().size(), 0);
}

/**
 * When the command is invoked it returns a value of zero
 */
TEST_F(AddCommandTest, ReturnValueOfSuccessfulAddCommandIsZero) {
    repo->writeFile("when.txt", "when");
    EXPECT_EQ(runAdd({"when.txt"}), 0);
}

/**
 * Ensure that a file that is in a subdirectory is added to the index
 * Example command:
 *                  gitty add dog/walk.groovy
 */
TEST_F(AddCommandTest, AddFileThatIsInSubdirectory) {
    repo->writeFile("dog/walk.groovy", "boolean walkies ?= true");
    runAdd({"dog/walk.groovy"});

    const auto index = loadIndex();
    ASSERT_EQ(index.getEntries().size(), 1);
    EXPECT_TRUE(index.isStaged("dog/walk.groovy"));
    EXPECT_TRUE(objectExists(index.getEntries().at("dog/walk.groovy").oid));
}

/**
 * Enure that a file which is deeply nested is added to the index
 * Example command:
 *                  gitty add i/go/outside/all/the/time.xml
 */
TEST_F(AddCommandTest, AddDeeplyNestedFile) {
    repo->writeFile("i/go/outside/all/the/time.xml", "i love trees");
    runAdd({"i/go/outside/all/the/time.xml"});

    const auto index = loadIndex();
    ASSERT_TRUE(index.isStaged("i/go/outside/all/the/time.xml"));
}

/**
 * Ensure that gitty add . skips files prefixed with .
 */
TEST_F(AddCommandTest, AddDotSkipsFilesThatAreDotFiles) {
    repo->writeFile(".hidden", "mypassword123");
    repo->writeFile("not_hidden.txt", "notmypassword123");
    runAdd({"."});

    auto index = loadIndex();
    EXPECT_FALSE(index.isStaged(".hidden"));
    EXPECT_TRUE(index.isStaged("not_hidden.txt"));
}

/**
 * Add a non-executable file to the index.
 */
TEST_F(AddCommandTest, NonExecutableFileMode) {
    repo->writeFile("normal.txt", "data");
    runAdd({"normal.txt"});

    auto index = loadIndex();
    EXPECT_EQ(index.getEntries().at("normal.txt").mode, 0100644);
}

/**
 * Check if executable files are okay to be added to the index
 */
TEST_F(AddCommandTest, ExecutableFileMode) {
    repo->writeFile("script.sh", "#!/bin/bash");
    makeExecutable("script.sh");
    runAdd({"script.sh"});

    auto index = loadIndex();
    EXPECT_EQ(index.getEntries().at("script.sh").mode, 0100755);
}

/**
 * Mixed permissions are okay to be added to the index
 */
TEST_F(AddCommandTest, MixedPermissionOkay) {
    repo->writeFile("normal.txt", "data");
    repo->writeFile("script.sh", "#!/bin/bash");
    makeExecutable("script.sh");
    runAdd({"normal.txt", "script.sh"});

    const auto index = loadIndex();
    EXPECT_EQ(index.getEntries().at("normal.txt").mode, 0100644);
    EXPECT_EQ(index.getEntries().at("script.sh").mode, 0100755);
}

/**
 * Ensure that the add command doesn't write the same file twice to the index.
 */
TEST_F(AddCommandTest, AddCommandSkipsStagedFiles) {
    repo->writeFile("file.txt", "file");
    runAdd({"file.txt"});

    const auto indexBefore  = loadIndex();
    std::string oidBefore= indexBefore.getEntries().at("file.txt").oid;
    int objectsBefore       = countObjects();

    runAdd({"file.txt"});

    const auto indexAfter = loadIndex();
    EXPECT_EQ(indexAfter.getEntries().size(), 1);
    EXPECT_EQ(indexAfter.getEntries().at("file.txt").oid, oidBefore);
    EXPECT_EQ(countObjects(), objectsBefore);
}

/**
 * When you modify a file the index should not update until you readd it to the index
 */
TEST_F(AddCommandTest, ModifiedFileDoesUpdateInTheIndex) {
    repo->writeFile("file.txt", "inside");
    runAdd({"file.txt"});
    const std::string oldOid = loadIndex().getEntries().at("file.txt").oid;

    repo->writeFile("file.txt", "outside");
    runAdd({"file.txt"});

    const auto newOid = loadIndex().getEntries().at("file.txt").oid;
    EXPECT_NE(oldOid, newOid);
}

TEST_F(AddCommandTest, MixOfStagedAndUnstaged) {
    repo->writeFile("old.txt", "old");
    runAdd({"old.txt"});
    const int objectsAfterFirst = countObjects();

    repo->writeFile("new.txt", "new");
    runAdd({"old.txt", "new.txt"});

    const auto index = loadIndex();
    EXPECT_EQ(index.getEntries().size(), 2);
    EXPECT_TRUE(index.isStaged("old.txt"));
    EXPECT_TRUE(index.isStaged("new.txt"));
    EXPECT_EQ(countObjects(), objectsAfterFirst + 1);
}

TEST_F(AddCommandTest, SameContentSameOid) {
    repo->writeFile("a.txt", "identical");
    repo->writeFile("b.txt", "identical");
    runAdd({"a.txt", "b.txt"});

    const auto index = loadIndex();
    EXPECT_EQ(index.getEntries().at("a.txt").oid, index.getEntries().at("b.txt").oid);
    EXPECT_EQ(countObjects(), 1);
}

TEST_F(AddCommandTest, DifferentContentDifferentOid) {
    repo->writeFile("a.txt", "aaa");
    repo->writeFile("b.txt", "bbb");
    runAdd({"a.txt", "b.txt"});

    const auto index = loadIndex();
    EXPECT_NE(index.getEntries().at("a.txt").oid, index.getEntries().at("b.txt").oid);
}

TEST_F(AddCommandTest, OidMatchesExpectedHash) {
    const std::string content = "i go outside";
    repo->writeFile("outside.txt", content);

    runAdd({"outside.txt"});

    const auto index = loadIndex();
    EXPECT_EQ(index.getEntries().at("outside.txt").oid, computeExpectedOid(content));
}

TEST_F(AddCommandTest, EmptyFileHasValidOid) {
    repo->writeFile("empty.txt", "");

    runAdd({"empty.txt"});

    const auto index = loadIndex();
    auto& entry = index.getEntries().at("empty.txt");
    EXPECT_EQ(entry.oid.length(), 40);
    EXPECT_EQ(index.getEntries().at("empty.txt").oid, computeExpectedOid(""));
    EXPECT_TRUE(objectExists(entry.oid));
}

TEST_F(AddCommandTest, BlobObjectCreatedOnDisk) {
    repo->writeFile("test.txt", "content");
    runAdd({"test.txt"});

    auto index = loadIndex();
    std::string oid = index.getEntries().at("test.txt").oid;

    fs::path objPath = getObjPath(*repo) / oid.substr(0, 2) / oid.substr(2);

    EXPECT_TRUE(fs::exists(objPath));
    EXPECT_GT(fs::file_size(objPath), 0);
}

TEST_F(AddCommandTest, ObjectDirectoryStructure) {
    repo->writeFile("test.txt", "content");
    runAdd({"test.txt"});

    const auto index = loadIndex();
    std::string oid = index.getEntries().at("test.txt").oid;

    fs::path fanoutDir = fs::path(repo->getPath()) / ".git" / "objects" / oid.substr(0, 2);
    EXPECT_TRUE(fs::is_directory(fanoutDir));
}

TEST_F(AddCommandTest, IndexFileCreatedOnDisk) {
    repo->writeFile("file.txt", "data");
    runAdd({"file.txt"});

    fs::path indexPath = fs::path(repo->getPath()) / ".git" / "index";
    EXPECT_TRUE(fs::exists(indexPath));
    EXPECT_GT(fs::file_size(indexPath), 0);
}

TEST_F(AddCommandTest, AddNonExistentFile) {
    const int result = runAdd({"ghost.txt"});
    EXPECT_NE(result, 0);
    EXPECT_EQ(countObjects(), 0);
}

/**
 * Staging a file then deleting it from disk and running add <path> should
 * remove the entry from the index (stage the deletion).
 */
TEST_F(AddCommandTest, AddDeletedTrackedFileStagesDeletion) {
    repo->writeFile("gone.txt", "content");
    runAdd({"gone.txt"});
    ASSERT_TRUE(loadIndex().isStaged("gone.txt"));

    fs::remove(fs::path(repo->getPath()) / "gone.txt");
    ASSERT_EQ(runAdd({"gone.txt"}), 0);

    EXPECT_FALSE(loadIndex().isStaged("gone.txt"));
}

/**
 * add . should stage the deletion of a tracked file that was removed from disk.
 */
TEST_F(AddCommandTest, AddDotStagesDeletionOfRemovedTrackedFile) {
    repo->writeFile("will_be_deleted.txt", "bye");
    repo->writeFile("stays.txt", "hello");
    runAdd({"will_be_deleted.txt", "stays.txt"});

    fs::remove(fs::path(repo->getPath()) / "will_be_deleted.txt");
    runAdd({"."});

    const auto index = loadIndex();
    EXPECT_FALSE(index.isStaged("will_be_deleted.txt")) << "Deleted file should be removed from index";
    EXPECT_TRUE(index.isStaged("stays.txt"));
}

/**
 * add <path> on a file that never existed and is not tracked should still
 * return non-zero (not silently succeed).
 */
TEST_F(AddCommandTest, AddNeverExistedUntrackedFileReturnsNonZero) {
    EXPECT_NE(runAdd({"never_existed.txt"}), 0);
}

TEST_F(AddCommandTest, AddWithNoArguments) {
    repo->writeFile("file.txt", "file");
    int result = runAdd({});

    EXPECT_EQ(result, 0);

    auto index = loadIndex();
    EXPECT_EQ(index.getEntries().size(), 0);
}