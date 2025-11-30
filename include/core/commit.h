#ifndef COMMIT_H 
#define COMMIT_H 

#include "core/git_object.h"
#include "core/author.h"
#include "string"

class Commit : public GitObject {
  private: 
    std::string treeOid; 
    std::string parentOid; 
    Author author; 
    Author committer; 
    std::string message; 

  public: 
    Commit(const std::string& tree, const std::string& parent, const Author& auth, const Author& commit, const std::string& msg);

    Commit(const std::string& tree, const std::string& parent, const Author& auth, const std::string& msg); 

    std::string getType() const override; 

    std::string getContent() const override; 

    std::string getTreeOid() const { return treeOid; }
    std::string getParentOid() const { return parentOid; }
    const Author& getAuthor() const { return author; }
    const Author& getCommitter() const { return committer; }
    std::string getMessage() const { return message; }

    std::string toString() const; 
}; 

#endif 
