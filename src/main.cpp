#include <iostream>
#include <string> 
#include <vector> 
#include "../include/commands/command_factory.h"

int main(int argc, char* argv[]) {
  CommandFactory::initializeCommands(); 

  std::vector<std::string> args(argv, argv + argc); 

  if (args.size() < 2) {
    std::cerr << "Usage: " << argv[0] << "<command>  [args...]" << std::endl;
    return 1;
  }

  std::string command_name = args[1]; 

  auto command = CommandFactory::create(command_name, args); 

  if (!command) {
    std::cerr << "jit: '" << command_name << "' is not a jit command." << std::endl;
    return 1; 
  }

  return command->execute(); 
}
