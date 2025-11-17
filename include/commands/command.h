#ifndef COMMAND_H
#define COMMAND_H 


#include <string>
#include <vector> 


class Command {
  public: 
    virtual ~Command()    = default;
    virtual int execute() = 0;
}; 


#endif
