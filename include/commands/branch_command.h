#ifndef BRANCH_COMMAND_H
#define BRANCH_COMMAND_H

#include "command.h"

class BranchCommand : public Command {
  private:
    bool deleteFlag;

    std::string getCurrentBranch(); 
    std::string getHeadCommit(); 
    void listBranches(); 
    void createBranch(const std::string& name);
    void deleteBranch(const std::string& name); 
  public: 
    BranchCommand();
    int execute() override; 
};



#endif 
