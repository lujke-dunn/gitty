#ifndef TREE_H
#define TREE_H

#include "core/git_object.h"
#include "core/entry.h"
#include <vector> 
#include <string>


class Tree : public GitObject {
  private: 
    std::vector<Entry> entries; 

  public: 
    explicit Tree(const std::vector<Entry>& treeEntries);

    std::string getType() const override; 

    std::string getContent() const override;

    void addEntry(const Entry& entry); 

    const std::vector<Entry>& getEntries() const { return entries; }

    std::string toString() const;
}

#endif
