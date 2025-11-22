#ifndef ENTRY_H
#define ENTRY_H 

#include <string> 
#include <filesystem> 

namespace fs = std::filesystem;

class Entry {
  private:
    std::string name;
    std::string oid; 
    int         mode;

  public: 
    Entry(const std::string& entryName, const std::string& entryOid, int entryMode); 

    const std::string& getName() const { return name; }
    const std::string& getOid()  const { return oid;  }
    const int getMode() const { return mode; }

    bool isTree() const { return mode == 04000 }; 

    std::string modeString() const; 

    bool operator<(const Entry& other)> const; 

    std::string toString() const; 
}

#endif
