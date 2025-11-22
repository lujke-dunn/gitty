#include "core/tree.h"
#include <algorithm>
#include <sstream> 

Tree::Tree(const std::vector<Entry>& treeEntries) : entries(treeEntries) {
  std::sort(entries.begin(), entries.end());
}

std::string Tree::getType() const {
  return "tree"; 
}

std::string Tree::getContent() const {
  std::string content; 

  for (const auto& entry : entries) {
    content += entry.modeString(); 
    content += " "; 
    content += entry.getName(); 
    content += '\0'; 
    
    std::string oid = entry.getOid(); 
    for (size_t i = 0; i < oid.length(); i += 2) {
      std::string byteString = oid.substr(i, 2); 
      char byte = static_cast<char>(std::stoi(byteString, nullptr, 16));
      content += byte; 
    }
  }

  return content; 
}

void Tree::addEntry(const Entry& entry) {
  entries.push_back(entry); 
  std::sort(entries.begin(), entries.end()); 
}

std::string Tree::toString() const {
  std::ostringstream oss; 
  for (const auto& entry : entries) {
    oss << entry.toString() << "\n";
  }
  return oss.str(); 
}
