#ifndef CONFIG_H
#define CONFIG_H 

#include <string>
#include <map>
#include <fstream> 
#include <sstream> 

class Config {
  private: 
    std::map<std::string, std::map<std::string, std::string>> data;
    std::string configPath;

    std::string trim(const std::string& str);
    void parseLine(const std::string& line, std::string& currentSection); 

  public: 
    Config(const std::string& path); 

    void load(); 
    std::string get(const std::string& section, const std::string& key); 
    bool has(const std::string& section, const std::string& key); 
    void set(const std::string& section, const std::string& key, const std::string& value);
    void save(); 
};

#endif
