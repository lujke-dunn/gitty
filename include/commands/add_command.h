#ifndef ADD_COMMAND_H
#define ADD_COMMAND_H

#include "commands/command.h"

// hmm so what do i want to do?
// add realistically has to do a few things 
// add things to the index based off user arguments, so we should probably check for the . argument first to see if they want to add something to the index
// we also have to identify if a given path to a thing is valid then add it? we also should check if said path is already in the index hmmm

class AddCommand : public Command { 
 public: 
  AddCommand(); 
  int execute() override; 
};
  
#endif 
