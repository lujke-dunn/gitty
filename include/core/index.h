#ifndef INDEX_H
#define INDEX_H

#include <string>
#include <map> 
#include <filesystem> 

namespace fs = std::filesystem; 

struct IndexEntry {
  std::string path; 
  std::string oid; 
  mode_t mode;

  IndexEntry() = default;
  IndexEntry(const std::string& p, const std::string& o, mode_t m) : path(p), oid(o), mode(m) {}
};

class Index {
  private:
    fs::path indexPath; 
    std::map<std::string, IndexEntry> entries;

    void load(); 
    void write(); 

  public:
    Index(const fs::path& gitPath);
    void add(const std::string& path, const std::string& oid, mode_t mode); 

    void remove(const std::string& path);

    bool isStaged(const std::string& path) const; 

    const std::map<std::string, IndexEntry>& getEntries() const; 

    void clear();
};

#endif 
