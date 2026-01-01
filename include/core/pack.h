#ifndef PACK_H
#define PACK_H

#include <string> 
#include <vector> 
#include <filesystem> 

namespace fs = std::filesystem;

/**
 * Generates a pack file for object transfer; pack files bundle multiple objects into a single binary for
 * efficient network transfer during push and fetch operations. Instead of sending individual files over the wire.
 * A pack file contains a header the objects and a checksum.
 *
 * The format of a pack file is the following
 *  - Header: "PACK", version, object count
 *  - Objects: each with a variable length header which defines the size and type.
 *  - Checksum: A SHA-1 hash of the entire pack's contents.
 *
 * This is a naive approach of a pack file with all of the contents as delta compression has not be implemented.
 */
class PackFile {
  private: 
    fs::path objectsPath; // Path to .git/objects
    std::vector<std::string> objectOids; // objects to pack

    /**
     * Writes object size and type in Git's variable length encoding.
     * First byte: [continue:1][type:3][size:4]
     * Subsequent bytes: [continue:1][size:7]
     *
     * @param out stream of output
     * @param size object content size
     * @param type object type code (1 = commit, 2 = tree, 3 = blob, 4 = tag)
     */
    void writePackedSize(std::ostream& out, size_t size, int type);

    /**
     * Calculates SHA-1 hash of data.
     *
     * @param data
     * @return raw 20-byte binary hash
     */
    std::string calculateSHA1(const std::string& data);

  public: 
    PackFile(const fs::path& gitPath);

    /**
     * Adds an object to be included in the pack.
     *
     * @param oid SHA-1 hash of object to pack
     */
    void addObject(const std::string& oid);

    /**
     * Reads all added objects, compresses them individually and bundles them into git pack format
     * with trailing checksum.
     *
     * @return Binary pack file data ready to be sent.
     */
    std::string generate();

    size_t getObjectCount() const { return objectOids.size(); }
}; 

#endif
