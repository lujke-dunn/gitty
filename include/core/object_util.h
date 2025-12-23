#ifndef OBJECT_UTILS_H
#define OBJECT_UTILS_H

#include <string> 
#include <utility> 
#include <filesystem> 

namespace fs = std::filesystem;

namespace ObjectUtils {
  std::string readObject(const fs::path& objectsPath, const std::string& oid); 

  std::pair<std::string, std::string> parseObject(const std::string& objectData); 

  std::string getObjectType(const std::string& objectData); 

  int getTypeInt(const std::string& type); 
}
  
#endif
