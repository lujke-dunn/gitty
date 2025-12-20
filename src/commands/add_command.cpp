#include "../../include/commands/add_command.h"
#include "../../include/commands/command_option.h"
#include "../../include/core/workspace.h"
#include "../../include/core/index.h"
#include "../../include/core/database.h"
#include "../../include/core/blob.h"
#include <filesystem>
#include <iostream>
#include <vector> 
#include <algorithm> 

namespace fs = std::filesystem;

AddCommand::AddCommand() {}



int AddCommand::execute() {
  Workspace workspace(rootPath);
  Database  database(dbPath);
  Index     index(gitPath); 
  
  if (positionalArgs.size() == 1 && positionalArgs[0] == ".") {
      positionalArgs = workspace.listFiles(); 
  }

  for (const auto& path : positionalArgs) {
    if (!index.isStaged(path)) {
      std::string data = workspace.readFile(path);
      Blob blob(data);
      database.store(blob); 
      auto perms  = fs::status(rootPath / path).permissions(); 
      mode_t mode = (perms & fs::perms::owner_exec) != fs::perms::none ? 0100755 : 0100644;
      
      index.add(path, blob.getOid(), mode); 
    } else {
      std::cout << path << "already in index" << std::endl; 
    }
    
  } 

  return 0;
}

