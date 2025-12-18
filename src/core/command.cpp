#include "../../include/commands/command.h"
#include <iostream> 

Command::Command() {
  setupPaths(); 
}

void Command::addOption(std::unique_ptr<CommandOption> option) {
  options.push_back(std::move(option)); 
}

std::string Command::getConfigValue(const std::string& section, const std::string& key, const std::string& defaultValue) {
  if (!config) return defaultValue; 
  std::string value = config->get(section, key); 
  return value.empty() ? defaultValue : value; 
}

std::string Command::getEditor() {
  std::string editor = getConfigValue("core", "editor"); 
  if (!editor.empty()) return editor;

  const char* git_editor = std::getenv("GIT_EDITOR");
  if (git_editor) return std::string(git_editor); 

  const char* visual = std::getenv("VISUAL"); 
  if (visual) return std::string(visual); 

  const char* editor_env = std::getenv("EDITOR"); 
  if (editor_env) return std::string(editor_env); 

  return "nvim"; 
}

void Command::setupPaths() {
  rootPath = fs::current_path(); 
  gitPath  = rootPath / ".git"; 
  dbPath   = gitPath / "objects"; 

  fs::path configPath = gitPath / "config"; 
  if (fs::exists(configPath)) {
    config = std::make_unique<Config>(configPath.string()); 
  }
}

void Command::parseArgs(int argc, char* argv[]) {
  // skip command name 
  int i = 1;

  while (i < argc) {
    std::string arg = argv[i]; 

    bool matched = false; 
    for (auto& option : options) {
      if (option->matches(arg)) {
        try {
          int consumed = option->parse(argc, argv, i); 
          i += consumed; 
          matched = true; 
          break; 
        } catch (const std::exception& e) {
          std::cerr << "error: " << e.what() << std::endl; 
          throw; 
        }
      }
    }
    
    if (!matched) {
      if (arg[0] == '-') {
        std::cerr << "error: unknown option: " << arg << std::endl; 
        throw std::runtime_error("Unknown option"); 
      }
      positionalArgs.push_back(arg);
      i++;
    }
  }
}

int Command::run(int argc, char* argv[]) {
  try {
    parseArgs(argc, argv); 
    return execute(); 
  } catch (const std::exception& e) {
    std::cerr << "fatal: " << e.what() << std::endl; 
    return 1; 
  }
}

void Command::showHelp() {
  std::cout << "Options:" << std::endl; 
  for (const auto& opt : options) {
    std::cout << "  "; 
    if (!opt->getShortFlag().empty()) {
      std::cout << opt->getShortFlag();
      if (!opt->getLongFlag().empty()) {
        std::cout << ", "; 
      }
    }
    if (!opt->getLongFlag().empty()) {
      std::cout << opt->getLongFlag();
    }
    std::cout << "\t" << opt->getHelp() << std::endl;
  }
}
