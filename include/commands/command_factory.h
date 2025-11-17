#ifndef COMMAND_FACTORY_H
#define COMMAND_FACTORY_H 

#include <string>
#include <vector> 
#include <memory>
#include <map> 
#include <functional> 
#include "command.h"


class CommandFactory {
  private:

    using CommandCreator = std::function<std::unique_ptr<Command>(const std::vector<std::string>&)>; 
    static std::map<std::string, CommandCreator> commands;

  public:

    static void registerCommand(const std::string& name, CommandCreator creator); 
    static std::unique_ptr<Command> create(
        const std::string& name, 
        const std::vector<std::string>& args
    );
    static void initializeCommands(); 

}; 

#endif 
