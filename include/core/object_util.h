#ifndef OBJECT_UTILS_H
#define OBJECT_UTILS_H

#include <string> 
#include <utility> 
#include <filesystem> 

namespace fs = std::filesystem;

/**
 * Utility functions for reading and parsing git objects from disk.
 */
namespace ObjectUtils {
  /**
   * Reads and decompresses a git object from disk.
   *
   * @param objectsPath the path to .git/objects directory
   * @param oid sha hash of object
   * @return decompressed object in the format: <type> <size>\0<content>
   */
  std::string readObject(const fs::path& objectsPath, const std::string& oid);

  /**
   * parses a git object with the expected format of <type> <size>\0<content> and
   * extracts the type and content, discarding size.
   *
   * @param objectData Raw object data
   * @return pair of [type, content] e.g. ("blob", "hey beautiful")
   */
  std::pair<std::string, std::string> parseObject(const std::string& objectData);

  /**
   * Converts git object type strings to an integer code.
   * Used for pack file encoding where types are represented as numbers
   *
   * Type codes:
   *  1 = commit
   *  2 = tree
   *  3 = blob
   *  4 = tag
   *
   * @param type Object type string ("commit", "tree", "blob", "tag")
   * @return Integer type code
   */
  int getTypeInt(const std::string& type);
}
  
#endif
