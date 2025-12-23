#include "../../include/commands/push_command.h"
#include "../../include/commands/command_option.h"
#include "../../include/core/remote.h"
#include "../../include/core/object_walker.h"
#include "../../include/core/pack.h"
#include "../../include/core/git_protocol.h"
#include <iostream>
#include <fstream> 
#include <cstdlib>

PushCommand::PushCommand() : force(false) {
  addOption(std::make_unique<BoolOption>(
        "-f", "--force", &force,
        "Force push (overwrite remote branch)"
  )); 
}

std::string PushCommand::getCurrentCommit(const std::string& branchName) {
  fs::path refPath = gitPath / "refs" / "heads" / branchName;

  if (!fs::exists(refPath)) {
    std::runtime_error("Branch '" + branchName + "' does not exist"); 
  }

  std::ifstream file(refPath); 
  std::string   commitOid; 
  std::getline(file, commitOid); 

  commitOid.erase(0, commitOid.find_first_not_of(" \t\r\n"));
  commitOid.erase(commitOid.find_last_not_of(" \t\r\n") + 1); 

  return commitOid;
}

std::string PushCommand::getAuthToken() {
  const char* envToken = std::getenv("GITHUB_TOKEN"); 
  if (envToken) {
    return std::string(envToken); 
  }

  std::string token = getConfigValue("github", "token"); 
  if (!token.empty()) {
    return token;
  }

  return "";
}

std::string PushCommand::getUsername() {
  const char* envUsername = std::getenv("GITHUB_USERNAME"); 
  if (envUsername) {
    return std::string(envUsername); 
  }

  std::string username = getConfigValue("github", "username"); 
  if (!username.empty()) {
    return username; 
  }

  return ""; 
}

int PushCommand::execute() {
  std::string remoteName = "origin"; 
  std::string branchName = "master"; 

  if (positionalArgs.size() >= 1) {
    remoteName = positionalArgs[0]; 
  }
  if (positionalArgs.size() >= 2) {
    branchName = positionalArgs[1];
  }

  try {
    Remote remote = Remote::fromConfig(*config, remoteName); 

    std::string currentCommit = getCurrentCommit(branchName); 

    std::string serviceUrl = remote.getServiceUrl("git-receive-pack");

    std::string username   =  getUsername(); 
    std::string authToken  =  getAuthToken(); 
    
    if (username.empty()) {
      std::cerr << "Warning: No username found" << std::endl; 
      std::cerr << "Set username in config file" << std::endl; 
    }

    if (authToken.empty()) {
      std::cerr << "Warning: No github token found" << std::endl;
      std::cerr << "Set token in config file" << std::endl; 
    }

    GitProtocol protocol(authToken, username); 

    std::cout << "Finding remote refs..." << std::endl; 
    auto remoteRefs = protocol.discoverRefs(serviceUrl); 

    std::string refName = "refs/heads/" + branchName;
    std::string oldOid  = std::string(40, '0'); 

    if (remoteRefs.find(refName) != remoteRefs.end()) {
      oldOid = remoteRefs[refName]; 

      if (oldOid == currentCommit) {
        std::cout << "Everything up-to-date" << std::endl; 
        return 0;
      }

      if (!force) {
        // TODO check if its a FF 
      }
    }

    ObjectWalker walker(gitPath); 
    auto objects = walker.collectObjects(currentCommit);
    std::cout << "Found " << objects.size() << " objects " << std::endl; 

    PackFile pack(gitPath); 

    for (const auto& oid : objects) {
      std::cout << "Adding objects: " << oid << std::endl;
      pack.addObject(oid); 
    }

    std::string packData = pack.generate(); 
    std::cout << "Pack size: " << packData.length() << " bytes" << std::endl; 
    
    std::ofstream packFile("/tmp/test.pack", std::ios::binary);
    packFile.write(packData.c_str(), packData.length()); 
    packFile.close(); 


    std::cout << "pack header (first 12 bytes)"; 
    for (int i = 0; i < 12 && i < packData.length(); i++) {
      printf("%02x ", (unsigned char)packData[i]);
    }
    
    std::cout << "about to call send pack" <<std::endl;
    std::cout << serviceUrl << " :serviceUrl" << std::endl; 
    std::cout << refName << " : refname " << std::endl; 
    std::cout << oldOid << " : oldOid" << std::endl;
    std::cout << currentCommit << " : current commiiit " << std::endl;
    std::cout << packData.length() << " : pack data length" << std::endl; 
    protocol.sendPack(serviceUrl, refName, oldOid, currentCommit, packData); 
    std::cout << "got send pack" << std::endl;

    std::cout << "To " << remote.getUrl() << std::endl;
    std::cout << "   " << oldOid.substr(0,7) << ".." << currentCommit.substr(0,7);
    std::cout << "   " << branchName << " -> " << branchName << std::endl; 

    return 0; 
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl; 
    return 1; 
  }
}
