#ifndef COMMIT_COMMAND_H
#define COMMIT_COMMAND_H

#include "commands/command.h"

class CommitCommand : public Command {
  public:
    CommitCommand() = default; 
    int execute()     override;
};

#endif 
