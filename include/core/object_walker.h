#ifndef OBJECT_WALKER_H
#define OBJECT_WALKER_H 

#include <string> 
#include <set>
#include <filesystem> 

namespace fs = std::filesystem;

class ObjectWalker {
  private: 
    fs::path objectsPath;
    std::set<std::string> visited; 

    void walkCommit(const std::string& commitOid, std::set<std::string>& objects); 
    void walkTree(const std::string& treeOid, std::set<std::string>& objects); 

  public:
    ObjectWalker(const fs::path& gitPath); 

    std::set<std::string> collectObjects(const std::string& have, const std::set<std::string>& want = {}); 
};

#endif 
