#ifndef TREE_H 
#define TREE_H

#include "core/tree.h"
#include "core/entry.h"
#include <vector> 
#include <string>
#include <memory>

class TreeBuilder { 
  public: 
    Tree* build(const std::vector<Entry>& entries); 

  private: 
    std::vector<std::string> getParentDirectories(const std::string& path) const; 

    std::string getBasename(const std::string& path) const; 

    void addEntry(Tree* tree, const std::vector<std::string>& parents, const Entry& entry); 
}; 


#endif
