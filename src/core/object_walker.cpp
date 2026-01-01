#include "../../include/core/object_walker.h"
#include "../../include/core/object_util.h"
#include <fstream>
#include <sstream>
#include <iomanip> 
#include <iostream> 
#include <zlib.h> 
#include <stdexcept>

ObjectWalker::ObjectWalker(const fs::path& gitPath) : objectsPath(gitPath / "objects") {}

void ObjectWalker::walkCommit(const std::string& commitOid, std::set<std::string>& objects) {
  if (visited.find(commitOid) != visited.end()) {
    return; 
  }
  visited.insert(commitOid); 

  objects.insert(commitOid); 

  std::string data = ObjectUtils::readObject(objectsPath, commitOid);

  // Parse the commit
  // "commit size\0tree <oid>\nparent <oid>\nauthor ... \ncommitter... \n\message"
  size_t nullPos = data.find('\0'); 
  if (nullPos == std::string::npos) {
    throw std::runtime_error("Invalid commit format"); 
  }

  std::string        content = data.substr(nullPos + 1); 
  std::istringstream stream(content); 
  std::string line; 

  while (std::getline(stream, line)) {
    if (line.empty()) break; 

    if (line.find("tree ") == 0) {
      // take tree oid 
      std::string treeOid = line.substr(5); 
      walkTree(treeOid, objects);
    } else if (line.find("parent ") == 0) {
      // take parent commit oid
      std::string parentOid = line.substr(7); 
      walkCommit(parentOid, objects); 
    }
     
  }
}

void ObjectWalker::walkTree(const std::string& treeOid, std::set<std::string>& objects) {
  if (visited.find(treeOid) != visited.end()) {
    return; 
  }
  visited.insert(treeOid); 

  objects.insert(treeOid); 

  std::string data = ObjectUtils::readObject(objectsPath, treeOid); 

  // tree has a format of tree size\0<mode> <name>\0<sha-of-20-bytes>
  size_t nullPos = data.find('\0'); 
  if (nullPos == std::string::npos) {
    throw std::runtime_error("Invalid tree format"); 
  }

  std::string content = data.substr(nullPos + 1); 
  size_t pos = 0; 

  while (pos < content.length()) {
    size_t spacePos = content.find(' ', pos);
    if (spacePos == std::string::npos) break; 

    std::string mode = content.substr(pos, spacePos - pos); 
    pos = spacePos + 1; 

    size_t nameNullPos = content.find('\0', pos);
    if (nameNullPos == std::string::npos) break; 

    std::string name = content.substr(pos, nameNullPos - pos); 
    pos = nameNullPos + 1; 

    if (pos + 20 > content.length()) break; 

    std::string sha1Binary = content.substr(pos, 20); 
    pos += 20; 

    std::stringstream ss; 
    
    // convert the sha1 binary to hex
    for (unsigned char c : sha1Binary) {
      // pad by 2 with '0' and convert those to hex
      ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c); 
    }
    std::string childOid = ss.str(); 

    if (mode == "40000") {
      // if its a tree walk it. 
      walkTree(childOid, objects); 
    } else {
      // this will be a blob (one should hope)
      if (visited.find(childOid) == visited.end()) {
        visited.insert(childOid); 
        objects.insert(childOid); 
      }
    }
  }
}

std::set<std::string> ObjectWalker::collectObjects(const std::string& have, const std::set<std::string>& want) {
  // ensure that visited list is clear so when we call this we get correct output 
  visited.clear(); 
  std::set<std::string> objects; 

  walkCommit(have, objects);

  if (!want.empty()) {
    std::cerr << "this does not work only works for fresh repos my friends" << std::endl; 
  }

  return objects; 
} 
