#include "../../include/commands/branch_command.h"
#include "../../include/commands/command_option.h"
#include "../../include/core/refs.h"
#include <iostream> 
#include <filesystem>

namespace fs = std::filesystem; 

BranchCommand::BranchCommand() {
  addOption(std::make_unique<BoolOption>(
    "-d", "--delete", &deleteFlag, "Commit message"
  ));

}

void BranchCommand::listBranches() {
  fs::path branchesDir = gitPath / "refs/heads";
  Refs refs(gitPath);

  std::string currentBranch = refs.getCurrentBranch();
  if (!fs::exists(branchesDir) || !fs::is_directory(branchesDir)) {
    std::cerr << "Error: no branches" << std::endl; 
    return; 
  }
  for (const auto& file : fs::directory_iterator(branchesDir)) {
    if (file.path().stem().string() == currentBranch) {
      std::cout << "* " << file.path().stem().string() << std::endl; 
    } else {
      std::cout << " " << file.path().stem().string() << std::endl;  
    }
  }
}


int BranchCommand::execute() {
  Refs refs(gitPath);

  if (positionalArgs.empty()) {
    listBranches();
    return 0; 
  }

  if (deleteFlag && !positionalArgs.empty()) {
    std::string branchName = positionalArgs[0];
    std::string currentBranch = refs.getCurrentBranch();

    if (branchName == currentBranch) {
      std::cerr << "Error: cannot delete current branch '" << branchName << "'" << std::endl;
      return 1;
    }

    refs.deleteBranch(branchName);
    std::cout << "Deleted branch '" << branchName << "'" << std::endl;
    return 0;
  }

  std::string branchName = positionalArgs[0];
  std::string headCommit    = refs.getHeadCommit();

  if (headCommit.empty()) {
    std::cerr << "Error: Cannot create branch - no commits yet" << std::endl;
    return 1;
  }

  refs.createBranch(branchName, headCommit);
  return 0; 
}