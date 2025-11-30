#include "entry.h"
#include <sstream>
#include <iomanip>

Entry::Entry(const std::string& entryName, const std::string& entryOid, int entryMode) : name(entryName), oid(entryOid), mode(entryMode) {}



// There are a bunch of different octal numbers which git uses to represent file types and permissions
// 100644 - Regular file with read / write permissions for the owner, and read-only for others (think a standard non-executable file)
// 100755 - Executable file which has 3 extra bits set which allows for groups to read and execute and owners to read, write, and set 
// e.g. file types include (.exe, .sh, etc.)
// 040000 - Directories (trees) points to another tree object containing more entries like (src/, includes/, etc.)
// 120000 - Symbolic link (symlink) points to another directory or file   
// octal numbers were probably chosen cause it maps well to unix permission bits like (rwx). 
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
