#ifndef INIT_COMMAND_H
#define INIT_COMMAND_H

#include "command.h"
#include <string> 

class InitCommand : public Command {
  private:
    std::string path; 

  public:
    explicit InitCommand(const std::string& path);
    int execute() override; 
}; 



#endif 
