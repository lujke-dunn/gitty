#include "../../include/core/config.h"
#include <algorithm>
#include <iostream>

Config::Config(const std::string& path) : configPath(path) {
  load(); 
}


std::string Config::trim(const std::string& str) {
  size_t start = str.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return ""; 
  }
  size_t end = str.find_last_not_of(" \t\r\n"); 
  return str.substr(start, end - start + 1); 
}

void Config::parseLine(const std::string& line, std::string& currentSection) {
  std::string trimmed = trim(line); 
  
  // skip empty lines and comments
  if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') {
    return; 
  }

  if (trimmed[0] == '[' && trimmed.back() == ']') {
    currentSection = trimmed.substr(1, trimmed.length() - 2); 
    currentSection = trim(currentSection);
    return; 
  }

  size_t equalPos = trimmed.find('=');
  if (equalPos != std::string::npos && !currentSection.empty()) {
    std::string key = trim(trimmed.substr(0, equalPos));
    std::string value = trim(trimmed.substr(equalPos + 1));

    data[currentSection][key] = value;
  }
}

void Config::load() {
  std::ifstream file(configPath);
  if (!file.is_open()) {
    return; 
  }

  std::string line;
  std::string currentSection;

  while (std::getline(file, line)) {
    parseLine(line, currentSection); 
  }

  file.close(); 
}


std::string Config::get(const std::string& section, const std::string& key) {
  auto sectionIt = data.find(section); 
  if (sectionIt == data.end()) {
    return ""; 
  }

  auto keyIt = sectionIt->second.find(key); 
  if (keyIt == sectionIt->second.end()) {
    return ""; 
  }

  return keyIt->second;
}

bool Config::has(const std::string& section, const std::string& key) {
  auto sectionIt = data.find(section); 
  if (sectionIt == data.end()) {
    return false;
  }
}

void Config::set(const std::string& section, const std::string& key, const std::string& value) {
  data[section][key] = value;
}

void Config::save() {
  std::ofstream file(configPath);
  if (!file.is_open()) {
    std::cerr << "Error: Could not write config file: " << configPath << std::endl; 
    return;
  }

  for (const auto& section : data) {
    file << "[" << section.first << "]" << std::endl; 
    for (const auto& keyValue : section.second) {
      file << "   " << keyValue.first << " = " << keyValue.second << std::endl;
    }

    file << std::endl;
  }

  file.close(); 
}

