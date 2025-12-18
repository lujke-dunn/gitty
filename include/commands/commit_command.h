#ifndef COMMIT_COMMAND_H
#define COMMIT_COMMAND_H

#include "commands/command.h"
#include "../core/author.h"

class CommitCommand : public Command {
  private: 
    std::string message;
    std::string authorOverride; 

    std::string getMessageFromEditor();
    Author getAuthor();
    std::string getParentCommit();
    void updateHead(const std::string& commitOid);

  public:
    CommitCommand(); 
    int execute()     override;
};

#endif 
