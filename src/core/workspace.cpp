#include "core/workspace.h"
#include <fstream>
#include <sstream>
#include <algorithm> 

Workspace::Workspace(const fs::path& path) : pathname(path) {}

std::vector<std::string> Workspace::listFiles() const {
  std::vector<std::string> files; 
  
  // TODO: make it so that this explores subdirectories 
  for (const auto& entry : fs::directory_iterator(pathname)) {
    if (!entry.is_regular_file()) { 
      continue; // FIXME: skipping directories as of right now
    }
    
    std::string filename = entry.path().filename().string(); 

    // Ignore .git and hidden files per git spec 
    if (filename == ".git") {
      continue; 
    }
    
    // Add the files back to the list 
    files.push_back(filename); 
  }

  // Git sorts the files alphabetically
  // naturally we do the same.
  std::sort(files.begin(), files.end()); 

  return files; 
}

std::string Workspace::readFile(const std::string& path) const {
  fs::path file_path = pathname / path; 

  std::ifstream file(file_path, std::ios::binary); 
  if (!file) {
    throw std::runtime_error("Cannot read file: " + path); 
  }

  std::stringstream buffer; 
  buffer << file.rdbuf(); 
  return buffer.str(); 

}


fs::file_status Workspace::statFile(const std::string& path) const {
  fs::path file_path = pathname / path;
  return fs::status(file_path); 
}
