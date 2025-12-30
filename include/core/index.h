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

  uint32_t ctime_sec;
  uint32_t ctime_nsec;
  uint32_t mtime_sec;
  uint32_t mtime_nsec;
  uint32_t dev;
  uint32_t ino;
  uint32_t uid;
  uint32_t gid;
  uint32_t size;

  IndexEntry() = default;
  IndexEntry(const std::string& p, const std::string& o, mode_t m)
  : path(p), oid(o), mode(m), ctime_sec(0), ctime_nsec(0), mtime_sec(0), mtime_nsec(0), dev(0),
  ino(0), uid(0), gid(0), size(0) {}
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
