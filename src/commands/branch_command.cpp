#include "../../include/commands/branch_command.h"
#include "../../include/commands/command_option.h"
#include <iostream> 
#include <fstream> 
#include <filesystem> 

namespace fs = std::filesystem; 

BranchCommand::BranchCommand() {
  addOption(std::make_unique<BoolOption>(
    "-d", "--delete", &deleteFlag, "Commit message"
  ));

}

std::string BranchCommand::getCurrentBranch() {
  fs::path headPath = gitPath / "HEAD";
  if (!fs::exists(headPath)) return ""; 

  std::ifstream file(headPath); 
  if (!file.is_open()) return "";

  std::string line;
  std::getline(file, line); 

  if (line.find("ref: refs/heads/") == 0) {
    std::string branch = line.substr(16);
    branch.erase(0, branch.find_first_not_of(" \t")); 
    branch.erase(branch.find_last_not_of(" \t\r\n") + 1);
    return branch;
  }
    

  return ""; 
  
}

std::string BranchCommand::getHeadCommit() {
  fs::path headPath = gitPath / "HEAD";
  if (!fs::exists(headPath)) return ""; 

  std::ifstream headFile(headPath);
  std::string   headContent;
  std::getline(headFile, headContent);

  if (headContent.find("ref:") == 0) {
    std::string refPath = headContent.substr(5); 
    refPath.erase(0, refPath.find_first_not_of(" \t")); 
    refPath.erase(refPath.find_last_not_of(" \t\r\n") + 1);

    fs::path refFilePath = gitPath / refPath;
    if (!fs::exists(refFilePath)) return "";

    std::ifstream refFile(refFilePath);
    std::string   parent;
    std::getline(refFile, parent);
    parent.erase(0, parent.find_first_not_of(" \t"));
    parent.erase(parent.find_last_not_of(" \t\r\n") + 1); 
    return parent;  
  }

  headContent.erase(0, headContent.find_first_not_of(" \t"));
  headContent.erase(headContent.find_last_not_of(" \t\r\n") + 1); 
  return headContent;
}

void BranchCommand::listBranches() {
  fs::path branchesDir = gitPath / "refs/heads";

  std::string currentBranch = getCurrentBranch(); 
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

void BranchCommand::createBranch(const std::string& name) {
  std::string headCommit = getHeadCommit();
  if (headCommit.empty()) {
    throw std::runtime_error("Cannot create branch - no commits yet"); 
  }

  fs::path branchPath = gitPath / "refs" / "heads" / name; 
  if (fs::exists(branchPath)) {
    throw std::runtime_error("Branch " + name + " already exists"); 
  }

  fs::create_directories(branchPath.parent_path()); 
  std::ofstream file(branchPath); 
  file << headCommit << std::endl; 
}

void BranchCommand::deleteBranch(const std::string& name) {
  std::string currentBranch = getCurrentBranch(); 
  if (name == currentBranch) {
    throw std::runtime_error("Cannot delete current branch"); 
  }

  fs::path branchPath = gitPath / "refs" / "heads" / name; 
  if (!fs::exists(branchPath)) {
    throw std::runtime_error("Branch '" + name + "' not found"); 
  }

  fs::remove(branchPath); 
  std::cout << "Deleted branch " << name << std::endl; 
}

int BranchCommand::execute() {

  if (positionalArgs.empty()) {
    listBranches();
    return 0; 
  }

  if (deleteFlag && !positionalArgs.empty()) {
    deleteBranch(positionalArgs[0]); 
    return 0; 
  }

  createBranch(positionalArgs[0]); 
  return 0; 
}
