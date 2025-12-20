#include "../../include/core/index.h"
#include <fstream>
#include <sstream> 
#include <iostream> 

Index::Index(const fs::path& gitPath) : indexPath(gitPath / "index") {
  if (fs::exists(indexPath)) {
    load(); 
  }
}

void Index::load() {
  std::ifstream file(indexPath); 
  if (!file.is_open()) return;

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string modeStr;
    std::string oid; 
    std::string path; 

    iss >> modeStr >> oid; 
    std::getline(iss, path); 

    if (!path.empty() && path[0] == ' ') {
      path = path.substr(1);
    }

    mode_t mode = std::stoi(modeStr, nullptr, 8); 
    entries[path] = IndexEntry(path, oid, mode); 
  }
}

void Index::write() {
  std::ofstream file(indexPath);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to write index file"); 
  }

  for (const auto& [path, entry] : entries) {
    file << std::oct << entry.mode << std::dec << " " << entry.oid << " " << entry.path << "\n"; 
  }

  file.close(); 
}

void Index::add(const std::string& path, const std::string& oid, mode_t mode) { 
  entries[path] = IndexEntry(path, oid, mode); 
  write(); 
}

void Index::remove(const std::string& path) {
  entries.erase(path); 
  write(); 
}

bool Index::isStaged(const std::string& path) const { 
  return entries.find(path) != entries.end();
}

const std::map<std::string, IndexEntry>& Index::getEntries() const {
  return entries;
}

void Index::clear() {
  entries.clear(); 
  write(); 
}
