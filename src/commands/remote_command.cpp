#include "../../include/commands/remote_command.h"
#include "../../include/commands/command_option.h"
#include "../../include/core/remote.h"
#include <iostream>
#include <fstream> 
#include <algorithm>
#include <functional>

RemoteCommand::RemoteCommand() {}

void RemoteCommand::listRemotes() {
  fs::path configPath = gitPath / "config"; 
  if (!fs::exists(configPath)) {
    return;
  }

  std::ifstream file(configPath);
  std::string   line;

  while (std::getline(file, line)) {
    if (line.find("[remote \"") == 0) {
      size_t start = line.find('"') + 1;
      size_t end   = line.find('"', start);
      std::string remoteName = line.substr(start, end - start); 

      std::cout << remoteName << std::endl;
    }
  }
}



void RemoteCommand::addRemote(const std::string& name, const std::string& url) {
  std::string section = "remote \"" + name + "\"";
  std::string existingUrl = config->get(section, "url"); 

  if (!existingUrl.empty()) {
    std::cerr << "error: remote: " << name << " already exists." << std::endl;
    return;
  }

  config->set(section, "url", url);
  config->set(section, "fetch", "+refs/heads/*:refs/remotes/" + name + "/*"); 
  config->save();

  std::cout << "Remote '" << name << "' added." << std::endl;
}

void RemoteCommand::removeRemote(const std::string& name) {
  fs::path configPath = gitPath / "config"; 
  if (!fs::exists(configPath)) {
    std::cerr << "error: No such remote: " << name << std::endl;
    return; 
  }

  std::ifstream     inFile(configPath); 
  std::stringstream buffer; 
  std::string       line;
  bool              inRemoteSection = false; 
  bool              foundRemote = false; 
  std::string       targetSection = "[remote \"" + name + "\"]";

  while (std::getline(inFile, line)) {
    if (line == targetSection) {
      inRemoteSection = true; 
      foundRemote     = true; 
      continue; 
    }

    if (inRemoteSection && !line.empty() && line[0] == '[') {
      inRemoteSection = false;
    }

    if (!inRemoteSection) {
      buffer << line << "\n"; 
    }
  }

  inFile.close(); 

  if (!foundRemote) {
    std::cerr << "error: No such remote: " << name << std::endl;
    return; 
  }

  std::ofstream outFile(configPath);
  outFile << buffer.str(); 
  outFile.close(); 

  std::cout << "Remote: '" << name << "' removed." << std::endl; 
} 

void RemoteCommand::renameRemote(const std::string& oldName, const std::string& newName) {
  std::string oldSection = "remote \"" + oldName + "\"";
  std::string url = config->get(oldSection, "url"); 

  if (url.empty()) {
    std::cerr << "error: no such remote: " << oldName << std::endl;
    return; 
  }

  std::string newSection  = "remote \"" + newName + "\"";
  std::string existingUrl = config->get(newSection, "url");

  if (!existingUrl.empty()) {
    std::cerr << "error: remote " << newName << " already exists." << std::endl;
    return;
  }

  std::string fetchSpec = config->get(oldSection, "fetch"); 

  config->set(newSection, "url", url); 
  if (!fetchSpec.empty()) {
    std::string updatedFetch = "+refs/heads/*:refs/remotes/" + newName + "/*"; 
    config->set(newSection, "fetch", updatedFetch); 
  }
  config->save(); 

  removeRemote(oldName); 

  std::cout << "Renamed remote '" << oldName << "' to '" << newName << "'" << std::endl;
}

void RemoteCommand::showRemote(const std::string& name) {
  try {
    Remote remote = Remote::fromConfig(*config, name); 

    std::cout << "* remote" << name << std::endl; 
    std::cout << "  Fetch URL:" << remote.getUrl() << std::endl;
    std::cout << "  Push  URL:" << remote.getUrl() << std::endl;

    if (!remote.getFetch().empty()) {
      std::cout << "  Remote branch: " << std::endl; 
      std::cout << "    " << remote.getFetch() << std::endl;
    }
  } catch (const std::exception& e) {
    std::cerr << "error: no such remote: " << name << std::endl;
  }
}


namespace {
  bool requireArgs(auto positionalArgs, size_t count, const std::string& usage) {
    if (positionalArgs.size() < count) {
      std::cerr << "usage: " << usage << std::endl; 
      return false;
    }
    return true;
  }
}



int RemoteCommand::execute() {
  std::map<std::string, std::function<int()>> subcommands = {
    {"add", [this]() 
      {
        if (!requireArgs(positionalArgs, 3, "remote add <name> <url>")) return 1; 
        addRemote(positionalArgs[1], positionalArgs[2]);
        return 0;
      }
    },
    {"remove", [this]() 
      {
        if (!requireArgs(positionalArgs, 2, "remote remove <name>")) return 1; 
        removeRemote(positionalArgs[1]);
        return 0; 
      }
    },
    {"rm", [this]() 
      {
        if (!requireArgs(positionalArgs, 2, "remote remove <name>")) return 1;
        removeRemote(positionalArgs[1]); 
        return 0; 
      }
    },
    {"rename", [this]() 
      {
        if (!requireArgs(positionalArgs, 3, "remote rename <old> <new>")) return 1; 
        renameRemote(positionalArgs[1], positionalArgs[2]); 
        return 0; 
      } 
    },
    {"show", [this]() 
      {
        if (!requireArgs(positionalArgs, 2, "remote show <name>")) return 1; 
        showRemote(positionalArgs[1]);
        return 0; 
      }
    }
  }; 

  if (positionalArgs.empty()) {
    listRemotes(); 
    return 0; 
  }
  
  std::string subcommand = positionalArgs[0]; 
  auto it = subcommands.find(subcommand); 

  if (it != subcommands.end()) {
      return it->second(); 
  };

  std::cerr << "error: Unknown subcommand: " << subcommand << std::endl;
  std::cerr << "usage: remote" << std::endl;
  std::cerr << "   or: remote add <name> <url>" << std::endl; 
  std::cerr << "   or: remote remove <name>" << std::endl; 
  std::cerr << "   or: remote rename <old> <new>" << std::endl; 
  std::cerr << "   or: remote show <name>" << std::endl; 
  return 1; 
}
