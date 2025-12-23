#ifndef PACK_H
#define PACK_H

#include <string> 
#include <vector> 
#include <filesystem> 

namespace fs = std::filesystem;

class PackFile {
  private: 
    fs::path objectsPath;
    std::vector<std::string> objectOids; 

    void writePackedSize(std::ostream& out, size_t size, int type); 

    std::string calculateSHA1(const std::string& data); 

  public: 
    PackFile(const fs::path& gitPath); 

    void addObject(const std::string& oid); 

    std::string generate(); 

    size_t getObjectCount() const { return objectOids.size(); }
}; 

#endif
