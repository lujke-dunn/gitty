#include "../../include/commands/command_factory.h"
#include "../../include/commands/init_command.h"
#include "../../include/commands/commit_command.h"

#include <iostream> 
#include <filesystem> 

namespace fs = std::filesystem;

std::map<std::string, CommandFactory::CommandCreator> CommandFactory::commands; 

void CommandFactory::registerCommand(const std::string& name, CommandCreator creator) {
  commands[name] = creator; 
}

void CommandFactory::initializeCommands() {

  registerCommand("init", [](const std::vector<std::string>& args) { 
    std::string path = (args.size() >= 3) ? args[2] : fs::current_path().string();
    return std::make_unique<InitCommand>(path); 
  }); 

  registerCommand("commit", [](const std::vector<std::string>& args) {
    return std::make_unique<CommitCommand>(); 
  }); 

}

std::unique_ptr<Command> CommandFactory::create(
  const std::string& name,
  const std::vector<std::string>& args
) {
  auto it = commands.find(name);
  if (it != commands.end()) {
    return it->second(args); 
  }
  return nullptr; 
}
