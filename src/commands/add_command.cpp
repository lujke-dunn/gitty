#include "../../include/commands/add_command.h"
#include "../../include/commands/command_option.h"
#include "../../include/core/workspace.h"
#include "../../include/core/index.h"
#include "../../include/core/database.h"
#include "../../include/core/blob.h"
#include <filesystem>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

namespace fs = std::filesystem;

AddCommand::AddCommand() {}



int AddCommand::execute() {
  Workspace workspace(rootPath);
  Database  database(dbPath);
  Index     index(gitPath); 
  
  if (positionalArgs.size() == 1 && positionalArgs[0] == ".") {
    auto workspaceFiles = workspace.listFiles();
    std::set<std::string> workspaceSet(workspaceFiles.begin(), workspaceFiles.end());

    // Stage new and modified files
    for (const auto& path : workspaceFiles) {
      std::string data = workspace.readFile(path);
      Blob blob(data);
      database.store(blob);
      auto perms  = fs::status(rootPath / path).permissions();
      mode_t mode = (perms & fs::perms::owner_exec) != fs::perms::none ? 0100755 : 0100644;
      index.add(path, blob.getOid(), mode);
    }

    // Stage deletions: tracked files no longer present in the workspace
    std::vector<std::string> toDelete;
    for (const auto& [path, entry] : index.getEntries()) {
      if (!workspaceSet.count(path)) {
        toDelete.push_back(path);
      }
    }
    for (const auto& path : toDelete) {
      index.remove(path);
    }

    return 0;
  }

  for (const auto& path : positionalArgs) {
    fs::path fullPath = rootPath / path;

    if (!fs::exists(fullPath)) {
      if (index.isStaged(path)) {
        index.remove(path);
      } else {
        std::cerr << "fatal: pathspec '" << path << "' did not match any files" << std::endl;
        return 1;
      }
      continue;
    }

    std::string data = workspace.readFile(path);
    Blob blob(data);
    database.store(blob);
    auto perms  = fs::status(fullPath).permissions();
    mode_t mode = (perms & fs::perms::owner_exec) != fs::perms::none ? 0100755 : 0100644;
    index.add(path, blob.getOid(), mode);
  }

  return 0;
}

