#ifndef DATABASE_H
#define DATABASE_H

#include <filesystem>
#include <string> 
#include "git_object.h"

namespace fs = std::filesystem;


class Database {
  private:
    fs::path pathname; 

    std::string hashContent(const std::string& content) const; 

    void writeObject(const std::string& oid, const std::string& content);

  public: 
    explicit Database(const fs::path& path);

    void store(GitObject& object); 

}; 

#endif
