#ifndef GIT_OBJECT_H
#define GIT_OBJECT_H 

#include <string>
#include <memory> 

class GitObject {
  protected: 
    std::string oid;
  
  public:
    virtual ~GitObject() = default; 

    virtual std::string getType() const = 0; 

    virtual std::string getContent() const = 0; 


    std::string serialize() const; 

    std::string getOid() const { return oid; }
    void setOid(const std::string& id) { oid = id; }
}

#endif
