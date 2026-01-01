#ifndef BLOB_H
#define BLOB_H

#include "core/git_object.h"
#include <string>

/**
 * A blob stores the contents of a file in the git object database.
 * It contains only file data, no filename or metadata.
 * An objects metadata is contained in the tree object which references blob by their hash.
 */

class Blob : public GitObject {
  private: 
    std::string data; 

  public: 
    explicit Blob(const std::string& data);

    std::string getType() const override;
    std::string getContent() const override; 
    
    const std::string& getData() const;
}; 

#endif
