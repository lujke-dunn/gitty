#ifndef TREE_H
#define TREE_H

#include "core/git_object.h"
#include "core/entry.h"
#include "core/blob.h"
#include <map>
#include <vector> 
#include <string>
#include <functional> 

/**
 * Represents a git tree (a snapshot of the directory).
 *
 * A tree stores directory contents as entries - each entry is either:
 *  - A blob (file) with mode, name, and oid
 *  - A subtree (subdirectory) with mode 040000, name, and oid
 *
 * Trees form a hierarchical structure representing the repository's directory layout.
 * Each tree can contain blobs and subtrees.
 *
 * Serialization format (getContent):
 *  For each entry: <mode> <name>\0<20-byte-binary-SHA>
 *  Entries are sorted according to the spec defined in the Entry::operator<
 */
class Tree : public GitObject {
  private: 
    std::map<std::string, Entry> blobEntries;  // Files in this directory

    std::map<std::string, Tree*> childTrees;   // subdirectories (owns the pointer yummers)

  public: 

    Tree() = default;
    ~Tree(); 

    std::string getType() const override;

    /**
     * Serializes tree in Git's binary format.
     * Format per entry: <mode> <name>\0<20-byte-binary-SHA>
     */
    std::string getContent() const override;

    /**
     * Adds a blob (file) to the tree.
     * @param entry not of type tree
     */
    void addBlob(const Entry& entry);

    /**
     * Adds a child tree (subdirectory).
     * Tree takes ownership of the pointer.
     */
    void addChildTree(const std::string& name, Tree* tree);

    /**
     * Gets existing child tree or creates new one.
     * Used for building nested directory structure.
     *
     * @return pointer to child tree
     */
    Tree* getOrCreateChildTree(const std::string& name);

    /**
     * Post order traversal of tree hierarchy.
     * Visits children before parents - ensuring child trees are processed before parents reference them.
     *
     * @param callback function called on each tree node
     */
    void traverse(std::function<void(Tree*)> callback);

    bool isEmpty() const { return blobEntries.empty() && childTrees.empty(); }
    
    std::string toString() const;
};

#endif
