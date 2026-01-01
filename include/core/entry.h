#ifndef ENTRY_H
#define ENTRY_H 

#include <string> 
#include <filesystem> 

namespace fs = std::filesystem;

/**
 * An entry is a member of a git tree.
 * It is either a file (blob) or a subdirectory (tree) within in a tree.
 * Each entry stores the name, object id, and unix file mode/permissions of the object allowing us to denote its type
 *
 * File modes:
 *  100644 - Regular non-executable file
 *  100755 - Executable file
 *  040000 - Directory (tree)
 *  120000 - symlink (don't have this yet because hard)
 */
class Entry {
  private:
    std::string name;
    std::string oid; 
    int         mode;

  public: 
    Entry(const std::string& entryName, const std::string& entryOid, int entryMode); 

    const std::string& getName() const { return name; }
    const std::string& getOid()  const { return oid;  }
    int getMode() const { return mode; }

    bool isTree() const { return mode == 040000; }


   /** There are a bunch of different octal numbers which git uses to represent file types and permissions
    * 100644 - Regular file with read / write permissions for the owner, and read-only for others (think a standard non-executable file)
    *  100755 - Executable file which has 3 extra bits set which allows for groups to read and execute and owners to read, write, and set
    *  e.g. file types include (.exe, .sh, etc.)
    *  040000 - Directories (trees) points to another tree object containing more entries like (src/, includes/, etc.)
    *  120000 - Symbolic link (symlink) points to another directory or file
    *  octal numbers were probably chosen cause it maps well to unix permission bits like (rwx).
    */
   std::string modeString() const;

    /**
     * Compares entries by file mode for sorting within a tree.
     * Git spec requires entries to be sorted in lexicographic order
     * with directories having '/' appended to the end ensuring idempotent sorting
     * allowing hashes to be calculated uniformly.
     *
     * @param other the entry it's being compared to
     * @return whether or not the entry has a higher precedence then the argument entry
     */
    bool operator<(const Entry& other) const;

    std::string toString() const; 
};

#endif
