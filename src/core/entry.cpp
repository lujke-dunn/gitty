#include "../../include/core/entry.h"
#include <sstream>
#include <iomanip>

Entry::Entry(const std::string& entryName, const std::string& entryOid, int entryMode) : name(entryName), oid(entryOid), mode(entryMode) {}




std::string Entry::modeString() const {
  std::ostringstream oss;
  oss << std::oct << mode; 
  return oss.str(); 
}

bool Entry::operator<(const Entry& other) const {
  std::string thisName  = getName();
  std::string otherName = other.getName(); 

  if (isTree()) {
    thisName += '/'; 
  }

  if (other.isTree()) {
    otherName += '/'; 
  }

  return thisName < otherName;
}

std::string Entry::toString() const {
  std::ostringstream oss;
  oss << modeString() << " " 
      << (isTree() ? "tree" : "blob") << " " 
      << getOid() << "\t" << getName();
  return oss.str(); 
}
