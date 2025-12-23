#include "../../include/core/object_util.h"
#include <fstream> 
#include <sstream>
#include <vector> 
#include <stdexcept>
#include <zlib.h>

namespace ObjectUtils {

  std::string readObject(const fs::path& objectsPath, const std::string& oid) {
    fs::path objPath = objectsPath / oid.substr(0, 2) / oid.substr(2); 

    if (!fs::exists(objPath)) {
      throw std::runtime_error("Object not found: " + oid); 
    }

    std::ifstream file(objPath, std::ios::binary); 
    std::stringstream compressedBuffer; 
    compressedBuffer << file.rdbuf(); 
    std::string compressed = compressedBuffer.str(); 

    uLongf decompressedSize = 1024 * 1024; 
    std::vector<unsigned char> decompressed(decompressedSize); 

    int result = uncompress(
      decompressed.data(),
      &decompressedSize, 
      reinterpret_cast<const unsigned char*>(compressed.c_str()),
      compressed.length()
    ); 

    if (result != Z_OK) {
      throw std::runtime_error("Failed to decompress object: " + oid); 
    }

    return std::string(reinterpret_cast<char*>(decompressed.data()), decompressedSize); 
  }

  std::pair<std::string, std::string> parseObject(const std::string& objectData) {
    size_t spacePos = objectData.find(' '); 
    if (spacePos == std::string::npos) {
      throw std::runtime_error("Invalid object format: no space found"); 
    }

    std::string type = objectData.substr(0, spacePos); 

    size_t nullPos = objectData.find('\0'); 
    if (nullPos == std::string::npos) {
      throw std::runtime_error("Invalid object format: no null byte found"); 
    }

    std::string content = objectData.substr(nullPos + 1); 

    return {type, content}; 
  }
  
  int getTypeInt(const std::string& type) {
    if (type == "commit") return 1; 
    if (type == "tree")   return 2; 
    if (type == "blob")   return 3;
    if (type == "tag")    return 4; 
    throw std::runtime_error("Unknown object type: " + type); 
  }

}

