#ifndef DATABASE_H
#define DATABASE_H

#include <filesystem>
#include <string> 
#include "git_object.h"

namespace fs = std::filesystem;

/**
 * Manages .git/objects/ and works as git's object database.
 *
 * This database writes git objects (blobs, trees, commits) using content addressable storage.
 * Objects are stored using the following format <type> <size>\0<content>.
 * the objects are then hashed using SHA-1 to generate an object id,
 * and then compressed using DEFLATE a lossless compression algorithm.
 *
 * The database structures the database using the format .git/objects/ab/cdef1234...
 * with the first two characters of the hash being a directory and the rest being the filename.
 *
 * This is done because most filesystems like ext4, or NTFS store directories as trees, so one big folder,
 * becomes many disk seeks even if using a ds like a b-tree, additionally large directories are hard to
 * cache destroying cache-locality deprecating seek time dramatically.
 *
 * the above bit might have to be fact checked a little bit that's just my understanding lmao
 */
class Database {
  private:
    fs::path pathname; // path to .git/objects

    std::string hashContent(const std::string& content) const; 

    void writeObject(const std::string& oid, const std::string& content);

  public:
    /**
     * Creates a db instance for given objects directory.
     * @param path to .git/objects
     */
    explicit Database(const fs::path& path);

    /**
     * Stores and formats a git object and places it in the objects directory.
     * This function computes the hash of the object, compresses the data, and writes to the disk.
     *
     * @param object the git object to modify and store
     */
    void store(GitObject& object);

}; 

#endif
