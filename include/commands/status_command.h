#ifndef STATUS_COMMAND_H
#define STATUS_COMMAND_H

#include "command.h"
#include "core/index.h"
#include <string>
#include <set>
#include <map>

class StatusCommand : public Command {
  private:
    std::map<std::string, std::string> getHeadTree();
    std::set<std::string> getWorkspaceFiles();
    bool fileChanged(const IndexEntry& entry);
  public:
    StatusCommand();
    int execute() override;
};

#endif