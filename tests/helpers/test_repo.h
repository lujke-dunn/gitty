#ifndef A_GIT_CLONE_TEST_REPO_H
#define A_GIT_CLONE_TEST_REPO_H
#include <string>

class TestRepo {
private:
    std::string tempPath;
public:
    TestRepo();
    ~TestRepo();

    std::string getPath();
    void writeFile(const std::string& path, const std::string& content);
    std::string readFile(const std::string& path);
};

#endif