#ifndef A_GIT_CLONE_LOG_COMMAND_H
#define A_GIT_CLONE_LOG_COMMAND_H

#include "command.h"
#include <string>

class LogCommand : public Command {
  private:
    void printCommit(const std::string& commitSha);
    std::string getParent(const std::string& commitContent);
    std::string formatDate(const std::string& authorDate);

  public:
    LogCommand();
    int execute() override;

};

#endif