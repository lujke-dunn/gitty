#include "../../include/commands/init_command.h"
#include <iostream> 
#include <filesystem> 
#include <vector> 

namespace fs = std::filesystem;  
  
InitCommand::InitCommand(const std::string& path) : path(path) {} 

int InitCommand::execute() {
  fs::path root_path = fs::absolute(path); 
  fs::path git_path  = root_path / ".git"; 

  std::vector<std::string> dirs = {"objects", "refs/heads"}; 

  for (const auto& dir : dirs) {
    try { 
      fs::create_directories(git_path / dir);
    } catch (const fs::filesystem_error& error) {
      std::cerr << "fatal: " << error.what() << std::endl; 
      return 1; 
    }
  }
  
  std::ofstream head_file(git_path / "HEAD"); 
  head_file << "ref: refs/heads/master" << std::endl; 
  head_file.close();

  Config config((git_path / "config").string()); 

  // TODO make these actually do somethign kinda just place holder rn so the file is created ig?
  config.set("core", "repositoryformatversion", "0");
  config.set("core", "filemode", "true"); 
  config.set("core", "bare", "false"); 
  config.save(); 

  std::cout << "Initialized empty git repository in " << git_path << std::endl; 
  return 0; 
}
