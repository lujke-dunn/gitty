#include "../../include/core/git_object.h"


std::string GitObject::serialize() const {
  std::string content = getContent(); 
  return getType() + " " + std::to_string(content.length()) + '\0' + content; 
};
