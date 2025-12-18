#ifndef COMMAND_H
#define COMMAND_H 

#include "command_option.h"
#include "../core/config.h"
#include <string>
#include <vector> 
#include <memory>
#include <filesystem> 

namespace fs = std::filesystem;

class Command {
  protected: 
    std::vector<std::unique_ptr<CommandOption>> options;
    std::vector<std::string> positionalArgs; 

    fs::path rootPath;
    fs::path gitPath; 
    fs::path dbPath; 

    std::unique_ptr<Config> config; 

    void addOption(std::unique_ptr<CommandOption> option); 
    std::string getConfigValue(const std::string& section, const std::string& key, const std::string& defaultValue = ""); 
    std::string getEditor(); 
    void setupPaths(); 

  public: 
    Command();
    virtual ~Command()    = default;

    void parseArgs(int argc, char* argv[]); 
    virtual int execute() = 0;
    int run(int argc, char* argv[]);
    void showHelp(); 
}; 

#endif
