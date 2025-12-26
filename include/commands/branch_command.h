#ifndef BRANCH_COMMAND_H
#define BRANCH_COMMAND_H

#include "command.h"

class BranchCommand : public Command {
  private:
    bool deleteFlag;

    void listBranches();

  public:
    BranchCommand();
    int execute() override; 
};



#endif 
