#include "core/tree.h"
#include <algorithm>
#include <sstream> 

Tree::~Tree() {
  for (auto& pair : childTrees) {
    delete pair.second; 
  }
}

std::string Tree::getType() const {
  return "tree"; 
}

std::string Tree::getContent() const {
  std::vector<Entry> allEntries; 
  
  // add blobs
  for (const auto& pair : blobEntries) {
    allEntries.push_back(pair.second); 
  }

  // add tree entries 
  for (const auto& pair : childTrees) {
    const std::string& name = pair.first; 
    Tree* childTree = pair.second;
    
    // create an entry representing this subdirectory
    // mode 04000 = directory, oid points to child tree 
    Entry treeEntry(name, childTree->getOid(), 04000); 
    allEntries.push_back(treeEntry); 
  }

  // sort all entries
  std::sort(allEntries.begin(), allEntries.end()); 

  std::string content; 

  for (const auto& entry : allEntries) {
    content += entry.modeString(); 
    content += " "; 

    content += entry.getName(); 
    content += '\0'; 

    std::string oid = entry.getOid(); 
    for (size_t i = 0; i < oid.length; i += 2) {
      std::string byteString = oid.substr(i, 2); 
      char byte = static_cast<char>(std::stoi(byteString, nullptr, 16));
      content += byte; 
    }
  }

  return content;
}

void Tree::addBlob(const Entry& entry) {
  blobEntries[entry.getName()] = entry;
}


void Tree::addChildTree(const std::string& name, Tree* tree) {
  childTrees[name] = tree; 
}

Tree* Tree::getOrCreateChildTree(const std::string& name) {
  auto it = childTrees.find(name); 
  if (it != childTrees.end()) {
    return it->second; 
  }

  Tree* child = new Tree(); 
  childTrees[name] = child; 
  return child; 
}

void Tree::traverse(std::function<void(Tree*)> callback) {
  for (auto& pair : childTrees) {
    pair.second->traverse(callback)
  }

  callback(this); 
}


std::string Tree::toString() const {
  std::ostringstream oss; 

  oss << "Tree (oid:" << getOid() << ")\n";

  for (const auto& pair : blobEntries) {
    oss << " " << pair.second.toString() << "\n"; 
  }

  for (const auto& pair : childTrees) {
    oss << "  040000 tree " << pair.second->getOid() << "\t" << pair.first << "/\n"; 
  }

  return oss.str(); 
}
