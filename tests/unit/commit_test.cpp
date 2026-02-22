#include "core/commit.h"

#include <charconv>
#include <iomanip>
#include <ranges>
#include <gtest/gtest.h>
#include <string>
#include <openssl/sha.h>

#include "core/author.h"


/**
 * Test format for commit object.
 *
 * Commit objects follow the below order.
 * @format tree <oid>\nparent <oid>\nauthor <info>\ncommiter <info>\n\n<message>\n
 * @see commit.h for further dissertation regarding commit object implementation
 * @see author.h for info about author format effects on the commit object
 *
 */
class CommitFormatTest : public ::testing::Test {
protected:
    std::string fakeTreeOid    = "88e38705fdbd3608cddbe904b67c731f3234c451";
    std::string fakeParentOid  = "2fb7e6b97a594fa7f9ccb927849e95c7c70e39f5";
    time_t      fixedTime      = 1512742583;

    Author makeAuthor(const std::string& name = "Test User", const std::string email = "test@example.com") const {
        return Author(name, email, fixedTime);
    }

    static std::vector<std::string> splitLines(const std::string& s) {
        std::vector<std::string> lines;
        std::istringstream       stream(s);
        std::string              line;
        while (std::getline(stream, line)) {
            lines.push_back(line);
        }

        return lines;
    }

    static std::string sha1(const std::string& input) {
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
        std::stringstream ss;
        for (const unsigned char i : hash) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(i);
        }

        return ss.str();
    }
};

TEST_F(CommitFormatTest, RootCommitHasNoParentLine) {
    const Author author = makeAuthor();
    const Commit commit(fakeTreeOid, "", author, "Initial Commit");
    const std::string content = commit.getContent();

    EXPECT_TRUE(content.find("parent") == std::string::npos) << "Root commit must not have a parent line";
}

TEST_F(CommitFormatTest, NonRootCommitLineOrder) {
    const Author author = makeAuthor();
    const Commit commit(fakeTreeOid, fakeParentOid, author, "Second commit");
    auto  lines = splitLines(commit.getContent());

    ASSERT_GE(lines.size(), 6);
    EXPECT_TRUE(lines[0].find("tree " == nullptr));
    EXPECT_TRUE(lines[1].find("parent " == nullptr));
    EXPECT_TRUE(lines[2].find("author " == nullptr));
    EXPECT_TRUE(lines[3].find("commiter" == nullptr));
    EXPECT_EQ(lines[4], "");
    EXPECT_EQ(lines[5], "Second commit");
}

TEST_F(CommitFormatTest, RootCommitLineOrder) {
    const Author author = makeAuthor();
    const Commit commit(fakeTreeOid, "", author, "Initial Commit");
    auto  lines  = splitLines(commit.getContent());

    ASSERT_GE(lines.size(), 5);
    EXPECT_TRUE(lines[0].find("tree " == nullptr));
    EXPECT_TRUE(lines[1].find("author " == nullptr));
    EXPECT_TRUE(lines[2].find("commiter " == nullptr));
    EXPECT_EQ(lines[3], "");
    EXPECT_EQ(lines[4], "Initial Commit");
}

TEST_F(CommitFormatTest, TreeLineContainsExactly40CharHex) {
    const Author author = makeAuthor();
    const Commit commit(fakeTreeOid, "", author, "msg");
    auto  lines = splitLines(commit.getContent());

    ASSERT_GE(lines.size(), 1);
    // "tree" + 40 hex chars = 45 chars
    std::string treeLine = lines[0];
    EXPECT_EQ(treeLine, "tree " + fakeTreeOid);
    EXPECT_EQ(treeLine.length(), 5 + 40);
}

TEST_F(CommitFormatTest, AuthorAndCommitterSameWhenSingleAuthor) {
    const Author author = makeAuthor();
    const Commit commit(fakeTreeOid, "", author, "msg");
    std::string content = commit.getContent();

    size_t authorPos    = content.find("author ");
    size_t committerPos = content.find("committer ");
    ASSERT_NE(authorPos, std::string::npos);
    ASSERT_NE(committerPos, std::string::npos);

    std::string authorLine    = content.substr(authorPos + 7, content.find('\n', authorPos) - authorPos - 7);
    std::string committerLine = content.substr(committerPos + 10, content.find('\n', committerPos) - committerPos - 10);

    EXPECT_EQ(authorLine, committerLine);
}

TEST_F(CommitFormatTest, SeperateAuthorAndCommitter) {
    const Author author("Writer", "writer@dev.com", fixedTime);
    const Author committer("Committer", "committer@dev.com", fixedTime + 100);
    const Commit commit(fakeTreeOid, "", author, committer, "patched commit");
    std::string content = commit.getContent();

    EXPECT_TRUE(content.find("author Writer <writer@dev.com>") != std::string::npos);
    EXPECT_TRUE(content.find("committer Committer <committer@dev.com>") != std::string::npos);
}

TEST_F(CommitFormatTest, MultiLineMessage) {
    const Author author = makeAuthor();
    const std::string multiMsg = "First line\n\nLong Description\n more description";
    const Commit commit(fakeTreeOid, "", author, multiMsg);
    std::string content = commit.getContent();

    EXPECT_TRUE(content.find("\n\n" + multiMsg + "\n") != std::string::npos);
}

TEST_F(CommitFormatTest, EmptyMessageStillHasTrailingNewLine) {
    const Author author = makeAuthor();
    const Commit commit(fakeTreeOid, "", author, "");
    const std::string content = commit.getContent();

    EXPECT_TRUE(content.back() == '\n');
}

TEST_F(CommitFormatTest, TypeIsCommit) {
    const Author author = makeAuthor();
    const Commit commit(fakeTreeOid, "", author, "msg");

    EXPECT_EQ(commit.getType(), "commit");
}