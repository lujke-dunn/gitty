#ifndef INDEX_H
#define INDEX_H

#include <string>
#include <map> 
#include <filesystem> 

namespace fs = std::filesystem;

/**
 * Represents a single entry in the git's index.
 *
 * Stores file path, object id, mode, and fs metadata.
 * The metadata (timestamps, inode, etc.) is used to detect if a file has changed
 * without rehashing it allowing for some optimization gains.
 *
 * TODO refactor this out so it is a subclass of entry
 */
struct IndexEntry {
  std::string path;   // relative path from repo root
  std::string oid;    // sha-1 hash of file content
  mode_t mode;        // unix file mode/permissions

  // fs metadata
  uint32_t ctime_sec;  // creation time in seconds
  uint32_t ctime_nsec; // creation time in nanoseconds
  uint32_t mtime_sec;  // modification time in seconds
  uint32_t mtime_nsec; // modification time in nanoseconds
  uint32_t dev;        // device id
  uint32_t ino;        // Inode number (contains meta data about this entry)
  uint32_t uid;        // user id
  uint32_t gid;        // group id
  uint32_t size;       // file size in bytes

  IndexEntry() = default;
  IndexEntry(const std::string& p, const std::string& o, mode_t m)
  : path(p), oid(o), mode(m), ctime_sec(0), ctime_nsec(0), mtime_sec(0), mtime_nsec(0), dev(0),
  ino(0), uid(0), gid(0), size(0) {}
};

class Index {
  private:
    fs::path indexPath; // path to .git/index
    std::map<std::string, IndexEntry> entries; // staged files

    /**
     * Loads index from disk
     * Parses binmary format, and validates signature and version.
     */
    void load();

    /**
     * Writes to index on disk
     * Serializes to binary format with padding and trailing SHA-1 checksum
     */
    void write();

  public:
    /**
     * Creates instance of index or loads existing index if present.
     *
     * @param gitPath path to .git directory
     */
    Index(const fs::path& gitPath);

    /**
     * Stages a file by adding it to the index.
     * stores the file and associated metadata in the index and then writes it to disk.
     *
     * @param path path to .git directory
     * @param oid sha hash of file contents
     * @param mode unix file mode/permissions
     */
    void add(const std::string& path, const std::string& oid, mode_t mode);

    /**
     * Removes a file from staging
     * @param path to .git directory
     */
    void remove(const std::string& path);

    /**
     * checks if a file is currently staged.
     *
     * @param path to .git directory
     * @return a bool which says if file is index or not
     */
    bool isStaged(const std::string& path) const;

    /**
     * Returns all staged entries
     */
    const std::map<std::string, IndexEntry>& getEntries() const;

    /**
     * Clears all staged files, and writes a empty index to disk.
     */
    void clear();
};

#endif 
