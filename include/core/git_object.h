#ifndef GIT_OBJECT_H
#define GIT_OBJECT_H 

#include <string>

/**
 * Abstract base class for all git objects (blobs, trees, commits, tags).
 * subclasses must implemnt getType() and getContent().
 */
class GitObject {
  protected: 
    std::string oid;
  
  public:
    virtual ~GitObject() = default; 

    virtual std::string getType() const = 0; 

    virtual std::string getContent() const = 0; 

    std::string getOid() const { return oid; }
    void setOid(const std::string& id) { oid = id; }
};

#endif
