#include "../../include/commands/commit_command.h"
#include "../../include/core/workspace.h"
#include "../../include/core/database.h"
#include "../../include/core/blob.h"
#include <iostream>
#include <filesystem> 

namespace fs = std::filesystem;

int CommitCommand::execute() {
  fs::path root_path = fs::current_path(); 
  fs::path git_path  = root_path / ".git"; 
  fs::path db_path   = git_path  / "objects"; 

  Workspace workspace(root_path);
  Database  database(db_path); 

  auto files = workspace.listFiles(); 

  std::cout << "Files in workspace:" << std::endl;
  for (const auto& file : files) {
    std::string data = workspace.readFile(file); 
    Blob blob(data);
    database.store(blob); 

    std::cout << " " << file << std::endl; 
  }

  return 0; 
}
