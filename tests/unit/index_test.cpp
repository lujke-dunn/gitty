#include <gtest/gtest.h>
#include "../helpers/test_repo.h"
#include <string>
#include "../include/core/index.h"
#include <fstream>
#include <openssl/sha.h>
#include <arpa/inet.h>
#include <array>
#include <cstring>

namespace fs = std::filesystem;

class IndexTest : public ::testing::Test {
protected:
    std::unique_ptr<TestRepo> repo;
    fs::path gitPath;

    void SetUp() override {
        repo    = std::make_unique<TestRepo>();
        gitPath = fs::path(repo->getPath()) / ".git";
        fs::create_directories(gitPath);

        std::string initCmd = "git init " + repo->getPath() + " 2>&1";
        system(initCmd.c_str());

    }

    fs::path indexPath() const {
        return gitPath / "index";
    }

    std::string readRawIndex() {
        std::ifstream file(indexPath(), std::ios::binary);
        std::stringstream buf;
        buf << file.rdbuf();
        return buf.str();
    }

    std::string fakeOid(int n) {
        std::string oid(40, '0');
        std::string num = std::to_string(n);
        for (size_t i = 0; i < num.size(); i++) {
            oid[40 - num.size() + i] = num[i];
        }
        return oid;
    }

    std::string runGit(const std::string& cmd) const {
        std::string gitDir = (fs::path(repo->getPath()) / ".git").string();
        std::string full = "git --git-dir=" + gitDir + " " + cmd + " 2>&1";
        std::array<char, 4096> buffer;
        std::string result;
        FILE* pipe = popen(full.c_str(), "r");
        if (!pipe) return "";
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
            result += buffer.data();
        pclose(pipe);
        return result;
    }
};

TEST_F(IndexTest, RoundTripForSingleEntry) {
    repo->writeFile("file.cpp", "code");

    {
        Index index(gitPath);
        index.add("file.cpp", fakeOid(1), 0100644);
    }

    Index loaded(gitPath);
    ASSERT_EQ(loaded.getEntries().size(), 1);
    EXPECT_EQ(loaded.getEntries().at("file.cpp").oid, fakeOid(1));
    EXPECT_EQ(loaded.getEntries().at("file.cpp").mode, 0100644);
}

TEST_F(IndexTest, RoundTripPreservesStatFields) {
    repo->writeFile("file.php", "php");

    uint32_t originalSize, originalMtime, originalCtime, originalIno;
    {
        Index index(gitPath);
        index.add("file.php", fakeOid(1), 0100644);
        const auto& entry = index.getEntries().at("file.php");
        originalSize   = entry.size;
        originalMtime  = entry.mtime_sec;
        originalCtime  = entry.ctime_sec;
        originalIno    = entry.ino;
    }

    Index loaded(gitPath);
    const auto& entry = loaded.getEntries().at("file.php");
    EXPECT_EQ(entry.size, originalSize);
    EXPECT_EQ(entry.mtime_sec, originalMtime);
    EXPECT_EQ(entry.ctime_sec, originalCtime);
    EXPECT_EQ(entry.ino, originalIno);
}

TEST_F(IndexTest, RoundTripAfterRemove) {
    repo->writeFile("first.txt", "I am first");
    repo->writeFile("second.xml", "I am a valid xml file");

    {
        Index index(gitPath);
        index.add("first.txt", fakeOid(1), 0100644);
        index.add("second.xml", fakeOid(2), 0100644);
        index.remove("first.txt");
    }

    Index loaded(gitPath);
    EXPECT_EQ(loaded.getEntries().size(), 1);
    EXPECT_FALSE(loaded.isStaged("first.txt"));
    EXPECT_TRUE(loaded.isStaged("second.xml"));
}

TEST_F(IndexTest, RoundTripAfterClear) {
    repo->writeFile("file.txt", "me file yes");
    {
        Index index(gitPath);
        index.add("file.txt", fakeOid(1), 0100644);
        index.clear();
    }

    Index loaded(gitPath);
    EXPECT_EQ(loaded.getEntries().size(), 0);
}

TEST_F(IndexTest, TrailingChecksumValid) {
    repo->writeFile("fie.txt", "data");
    {
        Index index(gitPath);
        index.add("fie.txt", fakeOid(1), 0100644);
    }

    std::string raw = readRawIndex();
    ASSERT_GT(raw.size(), 20u);

    std::string content    = raw.substr(0, raw.size() - 20);
    std::string storedHash = raw.substr(raw.size() - 20);

    unsigned char computed[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(content.data()), content.size(), computed);

    EXPECT_EQ(std::memcmp(computed, storedHash.data(), 20), 0);
}

TEST_F(IndexTest, EntryPadding8ByteAligned) {
    repo->writeFile("abc", "abc");
    repo->writeFile("abcde", "daphyduck");
    repo->writeFile("abcdef", "ronaldmcdonald");

    Index index(gitPath);
    index.add("abc", fakeOid(1), 0100644);
    index.add("abcde", fakeOid(2), 0100644);
    index.add("abcdef", fakeOid(3), 0100644);

    std::string raw = readRawIndex();
    size_t offset = 12;

    for (int i = 0; i < 3; i++) {
        uint16_t flags;
        std::memcpy(&flags, raw.data() + offset + 60, 2);
        uint16_t pathLen = ntohs(flags) & 0xFFF;

        size_t entrySize = 62 + pathLen;
        size_t padding   = (8 - (entrySize % 8)) % 8;
        offset += entrySize + padding;
    }

    EXPECT_EQ(offset, raw.size() - 20);
}

TEST_F(IndexTest, LoadsRejectsInvalidSignature) {
    std::ofstream file(indexPath(), std::ios::binary);
    file << "JUNK";
    file.close();

    Index index(gitPath);
    EXPECT_EQ(index.getEntries().size(), 0);
}

TEST_F(IndexTest, LoadRejectsUnsupportedVersion) {
    std::ofstream file(indexPath(), std::ios::binary);
    file.write("DIRC", 4);
    uint32_t version = htonl(3);
    file.write(reinterpret_cast<const char*>(&version), 4);
    file.close();

    Index index(gitPath);
    EXPECT_EQ(index.getEntries().size(), 0);
}

TEST_F(IndexTest, RealGitCanReadIndex) {
    repo->writeFile("hello.txt", "hello");
    repo->writeFile("src/main.cpp", "int main() {}");

    Index index(gitPath);
    index.add("hello.txt", fakeOid(1), 0100644);
    index.add("src/main.cpp", fakeOid(2), 0100644);

    std::string output = runGit("ls-files --stage");

    EXPECT_NE(output.find("hello.txt"), std::string::npos);
    EXPECT_NE(output.find("src/main.cpp"), std::string::npos);
    EXPECT_EQ(output.find("fatal: "), std::string::npos);
    EXPECT_EQ(output.find("error: "), std::string::npos);
}

TEST_F(IndexTest, RealGitReportsCorrectOids) {
    repo->writeFile("file.txt", "text");

    Index index(gitPath);
    index.add("file.txt", fakeOid(42), 0100644);

    std::string output = runGit("ls-files --stage");
    std::cout << output;
    EXPECT_NE(output.find(fakeOid(42)), std::string::npos);
}


