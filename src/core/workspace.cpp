#include "core/workspace.h"
#include <fstream>
#include <sstream>
#include <algorithm> 

Workspace::Workspace(const fs::path& path) : pathname(path) {}

std::vector<fs::path> Workspace::listFilesRecursively(const fs::path& dir) const {
  std::vector<fs::path> files; 

  try {
    for (const auto& entry : fs::directory_iterator(dir)) {
      std::string filename = entry.path().filename().string(); 

      if (filename[0] == '.' || filename == ".git") {
        continue; 
      }

      // we wanna recurse directories and subdirectories but ignore things like symlinks.. 
      if (entry.is_directory()) {
        // recurse subdirectories 
        auto subdir_files = listFilesRecursively(entry.path()); 
        files.insert(files.end(), subdir_files.begin(), subdir_files.end()); 
      } else if (entry.is_regular_file()) {
        fs::path relative = fs::relative(entry.path(), pathname); 
        files.push_back(relative);
      }
    }
  } catch (const fs::filesystem_error& e) {
    // just do nothing when we can't read a directory; no need to crash. 
  }

  return files; 
}



std::vector<std::string> Workspace::listFiles() const {
  
  auto file_paths = listFilesRecursively(pathname);

  std::vector<std::string> files; 
  // probably decent idea to make the vector have a pre-allocated size.
  // since we already know how many items are in the directory already. 
  files.reserve(file_paths.size()); 

  for (const auto& path : file_paths) {
    files.push_back(path.string()); 
  }

  // git sorts the files alphabetically, meaning we also have to. 
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
