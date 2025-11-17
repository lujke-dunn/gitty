#ifndef BLOB_H
#define BLOB_H

#include "core/git_object.h"
#include <string>

class Blob : public GitObject {
  private: 
    std::string data; 

  public: 
    explicit Blob(const std::string& data); 

    std::string getType() const override; 
    std::string getContent() const override; 
    
    std::string toString() const;
    std::string& getData() const; 
}; 



#endif 
