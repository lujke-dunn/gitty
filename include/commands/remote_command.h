#ifndef REMOTE_COMMAND_H 
#define REMOTE_COMMAND_H 

#include "commands/command.h"
#include <string> 

class RemoteCommand : public Command { 
  private: 
    void listRemotes();
    void addRemote(const std::string& name, const std::string& url); 
    void removeRemote(const std::string& name);
    void renameRemote(const std::string& oldName, const std::string& newName); 
    void showRemote(const std::string& name); 
  public: 
    RemoteCommand(); 
    int execute() override; 
};

#endif
