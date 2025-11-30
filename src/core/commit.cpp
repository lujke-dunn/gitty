#include "../../include/core/commit.h"
#include <sstream>

Commit::Commit(const std::string& tree, const std::string& parent, const Author& auth, const Author& commit, const std::string& msg) :
  treeOid(tree), parentOid(parent), author(auth), committer(commit), message(msg) {}

Commit::Commit(const std::string& tree, const std::string& parent, const Author& auth, const std::string& msg) : 
  treeOid(tree), parentOid(parent), author(auth), committer(commit), message(msg) {}

std::string Commit::getType() const {
  return "commit"; 
}

std::string Commit::getContent() const {
  std::ostringstream oss; 
  oss << "tree " << treeOid << "\n";

  if (!parentOid.empty()) {
    oss << "parent " << parentOid << "\n";
  }

  oss << "author " << author.toString() << "\n";

  oss << "commiter " << commiter.toString() << "\n";

  oss << message;

  return oss.str(); 
}

std::string Commit::toString() const {
  std::ostringstream oss;  
  oss << "Commit " << getOid() << "\n"; 
  oss << "Tree " << treeOid << "\n"; 
  if (!parentOid.empty()) {
    oss << "Parent: " << parentOid << "\n";
  }

  oss << "Author: " << author.toString() << "\n"; 
  oss << "Commiter: " << commiter.toString() << "\n"; 
  oss << "\n" << message;
  return oss.str(); 
}
