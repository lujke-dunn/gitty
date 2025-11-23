#include "../../include/core/tree_builder.h"
#include <algorithm> 
#include <sstream> 


Tree* TreeBuilder::build(const std::vector<Entry>& entries) {
  Tree* root = new Tree(); 
  

  // TODO: fix the naming of these variables kinda not descriptive
  std::vector<Entry> sortedEntries = entries; 
  std::sort(sortedEntries.begin(), sortedEntries.end(), [](const Entry& a, const Entry& b) {
      return a.getName() < b.getName(); 
    });

  for (const Entry& entry: sortedEntries) {
    std::vector<std::string> parents = getParentDirectories(entry.getName()); 
    addEntry(root, parents, entry);
  }

  return root;
}

std::vector<std::string> TreeBuilder::getParentDirectories(const std::string& path) const {
  std::vector<std::string> parents; 

  size_t pos = 0;
  while ((pos = path.find('/', pos)) != std::string::npos) {
    parents.push_back(path.substr(0, pos)); 
    pos++; 
  }

  return parents; 
}

std::string TreeBuilder::getBasename(const std::string& path) const {
  size_t pos = path.find_last_of('/'); 
  if (pos == std::string::npos) {
    return path; // return whole path without forward slash
  }
  return path.substr(pos + 1); 
}

void TreeBuilder::addEntry(Tree* tree, const std::vector<std::string>& parents, const Entry& entry) {
  if (parents.empty()) {
    std::string basename = getBasename(entry.getName()); 
    Entry blobEntry(basename, entry.getOid(), entry.getMode()); 
    tree->addBlob(blobEntry); 
  } else {
    std::string firstParent = parents[0]; 
    std::string parentBasename = getBasename(firstParent); 
    
    Tree* childTree = tree->getOrCreateChild(parentBasename); 

    std::vector<std::string> remainingParents(parents.begin() + 1, parents.end()); 
    addEntry(childTree, remainingParents, entry); 
  }
}
