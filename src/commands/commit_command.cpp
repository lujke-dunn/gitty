#include "../../include/commands/commit_command.h"
#include "../../include/commands/command_option.h"
#include "../../include/core/workspace.h"
#include "../../include/core/database.h"
#include "../../include/core/blob.h"
#include "../../include/core/tree.h"
#include "../../include/core/tree_builder.h"
#include "../../include/core/entry.h"
#include "../../include/core/author.h"
#include "../../include/core/commit.h"
#include "../../include/core/config.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector> 

namespace fs = std::filesystem;

CommitCommand::CommitCommand() {
  addOption(std::make_unique<StringOption>(
    "-m", "--message", &message, "Commit message"
  ));

  addOption(std::make_unique<StringOption>(
    "", "--author", &authorOverride, "Override author (format: \"Name <email>\")"
  ));
}

std::string CommitCommand::getMessageFromEditor() {
  fs::path commitMsgFile = gitPath / "COMMIT_EDITMSG";

  std::ofstream msgFile(commitMsgFile); 
  msgFile << "\n";
  msgFile << "# Please enter your commit message for changes. lines starting with # are ignored";
  msgFile << "# An empty commit message aborts the commit";
  msgFile.close();

  std::string editor  = getEditor();
  std::string command = editor + " " + commitMsgFile.string(); 
  int result = system(command.c_str());

  if (result != 0) {
    throw std::runtime_error("editor exited with non-zero status"); 
  }

  std::ifstream readFile(commitMsgFile); 
  std::string msg; 
  std::string line; 

  while (std::getline(readFile, line)) {
    if (!line.empty() && line[0] == '#') continue;
    msg += line + "\n"; 
  }
  readFile.close(); 

  while (!msg.empty() && (msg.back() == '\n' || msg.back() == ' ' || msg.back() == '\t')) {
    msg.pop_back(); 
  }

  return msg;
}

Author CommitCommand::getAuthor() {
  std::string name;
  std::string email; 

  if (!authorOverride.empty()) { 
    size_t start = authorOverride.find('<'); 
    size_t end   = authorOverride.find('>'); 

    if (start == std::string::npos || end == std::string::npos) {
      throw std::runtime_error("Bad author format"); 
    }
    
    name  = authorOverride.substr(0, start); 
    email = authorOverride.substr(start + 1, end - start - 1);

    name.erase(0, name.find_first_not_of(" \t"));
    name.erase(name.find_last_not_of(" \t") + 1); 
    email.erase(0, name.find_first_not_of(" \t"));
    email.erase(name.find_last_not_of(" \t") + 1); 
 
  } else {
    name  = getConfigValue("user", "name");
    email = getConfigValue("user", "email");

    if (name.empty() || email.empty()) {
      throw std::runtime_error(
          "please configure user.name and user.email"
      );
    }
  }

  return Author(name, email); 
}

std::string CommitCommand::getParentCommit() {
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

void CommitCommand::updateHead(const std::string& commitOid) {
  fs::path masterRef = gitPath / "refs" / "heads" / "master";
  fs::create_directories(masterRef.parent_path());

  std::ofstream refFile(masterRef); 
  refFile << commitOid << std::endl; 
}

int CommitCommand::execute() {
  std::string commitMessage;
  if (!message.empty()) {
    commitMessage = message; 
  } else {
    commitMessage = getMessageFromEditor();
  }

  if (commitMessage.empty()) {
    std::cerr << "Aborting commit due to empty commit message" << std::endl; 
    return 1; 
  }

  Workspace workspace(rootPath);
  Database  database(dbPath);

  auto files = workspace.listFiles(); 
  std::vector<Entry> entries; 

  for (const auto& file : files) {
    std::string data = workspace.readFile(file);
    Blob blob(data); 
    database.store(blob);

    fs::path fullPath = rootPath / file; 
    auto perms  = fs::status(fullPath).permissions(); 
    mode_t mode = (perms & fs::perms::owner_exec) != fs::perms::none ? 0100755 : 0100644;

    Entry entry(file, blob.getOid(), mode);
    entries.push_back(entry); 

    std::cout << " blob " << blob.getOid() << " " << file << std::endl;
  }
  
  TreeBuilder builder;
  Tree* root = builder.build(entries); 

  root->traverse([&](Tree* tree) {
    database.store(*tree);
    std::cout << " tree " << tree->getOid() << std::endl;
  });

  std::string parent = getParentCommit(); 
  Author author = getAuthor(); 

  Commit commit(root->getOid(), parent, author, commitMessage);
  database.store(commit); 

  std::cout << "[master";
  if (parent.empty()) {
    std::cout << " (root-commit)";
  }
  std::cout << " " << commit.getOid().substr(0, 7) << "] ";
  
  size_t firstNewLine = commitMessage.find("\n"); 
  if (firstNewLine != std::string::npos) {
    std::cout << commitMessage.substr(0, firstNewLine);
  } else {
    std::cout << commitMessage.substr(0, 50); 
    if (commitMessage.length() > 50) {
      std::cout << "..."; 
    }
  }
  std::cout << std::endl;

  updateHead(commit.getOid()); 

  delete root; 

  return 0;
}
