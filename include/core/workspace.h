#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <filesystem>
#include <string>
#include <vector>


namespace fs = std::filesystem; 

class Workspace {
  private:
    fs::path pathname; 

    std::vector<fs::path> listFilesRecursively(const fs::path& dir) const;

  public:
    explicit Workspace(const fs::path& path);

    std::vector<std::string> listFiles() const; 

    std::string readFile(const std::string& path) const; 

    fs::file_status statFile(const std::string& path) const; 
};

#endif
