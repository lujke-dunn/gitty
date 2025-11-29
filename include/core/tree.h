#ifndef TREE_H
#define TREE_H

#include "core/git_object.h"
#include "core/entry.h"
#include "core/blob.h"
#include <map>
#include <vector> 
#include <string>
#include <functional> 


class Tree : public GitObject {
  private: 
    std::map<std::string, Entry> blobEntries; 

    std::map<std::string, Tree*> childTrees;  

  public: 

    Tree() = default;
    ~Tree(); 

    std::string getType() const override; 

    std::string getContent() const override;
    
    void addBlob(const Entry& entry); 

    void addChildTree(const std::string& name, Tree* tree); 

    Tree* getOrCreateChild(const std::string& name); 

    void traverse(std::function<void(Tree*)> callback); 

    bool isEmpty() const { return blobEntries.empty() && childTrees.empty(); }
    
    std::string toString() const;
}

#endif
