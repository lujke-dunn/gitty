#ifndef OBJECT_WALKER_H
#define OBJECT_WALKER_H 

#include <string> 
#include <set>
#include <filesystem> 

namespace fs = std::filesystem;

/**
 * Traverses git's object graph to collect all objects reachable in a commit
 *
 * This is used to generate pack files during push operations, and performs a depth first traversal
 * starting from a commit, following tree and parent references to collect all reachable objects (commits, trees,
 * and blobs).
 *
 * TODO fix me we currently push everything like a band of monkeys breaking push!
 */
class ObjectWalker {
  private: 
    fs::path objectsPath;          // path to .git/objects
    std::set<std::string> visited; // tracks visited oids to prevent cycles

    /**
     * Recursively walks a commit and it's parents.
     * Adds commit oid and walks its tree. following parent commits.
     *
     * @param commitOid
     * @param objects the set of objects associated with a commit
     */
    void walkCommit(const std::string& commitOid, std::set<std::string>& objects);

    /**
     * Recursively walks a tree and it's subtrees.
     * Parses tree entries, and recursively walks subdirectories, adding blob oid's
     *
     * @param treeOid
     * @param objects the set of objects within a tree
     */
    void walkTree(const std::string& treeOid, std::set<std::string>& objects);

  public:
    ObjectWalker(const fs::path& gitPath);

    /**
     * Collects all reachable objects from a commit by performing a traversal from the want commit.
     *
     * @param have set of oid's already on the remote TODO fix this i broke it hahah
     * @param want oid of the commit to send
     * @return set of all object oid's that need to be sent
     */
    std::set<std::string> collectObjects(const std::string& have, const std::set<std::string>& want = {});
};

#endif 
