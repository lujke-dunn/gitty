#include "../../include/commands/command_option.h"
#include <stdexcept>

CommandOption::CommandOption(const std::string& shortF, const std::string& longF, const std::string& help) : shortFlag(shortF), longFlag(longF), helpText(help) {}

bool CommandOption::matches(const std::string& arg) const {
  return (!shortFlag.empty() && arg == shortFlag) || (!longFlag.empty() && arg == longFlag); 
}

StringOption::StringOption(const std::string& shortF, const std::string& longF, std::string* val, const std::string& help) : CommandOption(shortF, longF, help), value(val), wasSet(false) {}

int StringOption::parse(int argc, char* argv[], int currentPos) {
  if (currentPos + 1 >= argc) {
    throw std::runtime_error("Option " + std::string(argv[currentPos]) + " requires a value"); 
  }

  *value = argv[currentPos + 1]; 
  wasSet = true;
  return 2; 
}

bool StringOption::isSet() const {
  return wasSet;
}

BoolOption::BoolOption(const std::string& shortF, const std::string& longF, bool* val, const std::string& help) : CommandOption(shortF, longF, help), value(val) {
  *value = false; 
}

int BoolOption::parse(int argc, char* argv[], int currentPos) {
  *value = true;
  return 1;
}

bool BoolOption::isSet() const {
  return *value;
}

IntOption::IntOption(const std::string& shortF, const std::string& longF, int* val, const std::string& help) : CommandOption(shortF, longF, help), value(val), wasSet(false) {}

int IntOption::parse(int argc, char* argv[], int currentPos) {
  if (currentPos + 1 >= argc) {
    throw std::runtime_error("Option " + std::string(argv[currentPos]) + " requires a value"); 
  }
  try {
    *value = std::stoi(argv[currentPos + 1]); 
    wasSet = true; 
  } catch (const std::exception& e) {
    throw std::runtime_error("Invalid integer value for " + std::string(argv[currentPos]));
  }

  return 2; 
}

bool IntOption::isSet() const {
  return wasSet; 
}

MultiStringOption::MultiStringOption(const std::string& shortF, const std::string& longF, std::vector<std::string>* vals, const std::string& help) : CommandOption(shortF, longF, help), values(vals) {}

int MultiStringOption::parse(int argc, char* argv[], int currentPos) {
  if (currentPos + 1 >= argc) {
    throw std::runtime_error("Option " + std::string(argv[currentPos]) + " requires a value"); 
  }
  values->push_back(argv[currentPos + 1]); 
  return 2;
}

bool MultiStringOption::isSet() const {
  return !values->empty(); 
}
