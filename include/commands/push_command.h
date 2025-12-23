#ifndef PUSH_COMMAND_H
#define PUSH_COMMAND_H

#include "command.h"
#include <string> 

class PushCommand : public Command {
  private:
    bool force;
    
    std::string getCurrentCommit(const std::string& branchName); 
    std::string getUsername(); 
    std::string getAuthToken(); 

  public:
    PushCommand(); 
    int execute() override;
};

#endif
