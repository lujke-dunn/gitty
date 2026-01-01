#ifndef COMMIT_H 
#define COMMIT_H 

#include "core/git_object.h"
#include "core/author.h"
#include "string"


/**
 * A commit is a snapshot of the repository by referencing a tree object.
 * Commits are composed of reference to a tree which contains the state of the blobs at that given time.
 * Additionally commits typically link to a parent commit which form a history of changes to a codebase over time.
 * Commits contain some additionally metadata including the author, committer (which is the same, unless applying a patch),
 * and a message set by the user which typically makes notes of the changes in a given commit.
 */
class Commit : public GitObject {
  private: 
    std::string treeOid;   // hash of the tree object (snapshot of the repo)
    std::string parentOid; // hash of the parent commit (empty for root commit)
    Author author;         // person who wrote the changes
    Author committer;      // person who committed the changes
    std::string message;   // optional message (when using --allow-empty)

  public:
    /**
     * Creates a commit with a separate author and committer.
     * Typically only used when applying a patch or otherwise specified via command args.
     *
     * @param tree sha-1 hash of tree associated with commit
     * @param parent parent sha-1 commit hash
     * @param auth author of the commit
     * @param commit the person who applied the change
     * @param msg commit message
     */
    Commit(const std::string& tree, const std::string& parent, const Author& auth, const Author& commit, const std::string& msg);

    /**
     * Creates a commit where author and committer are the same (the typical case).
     *
     * @param tree sha-1 hash of tree associated with commit
     * @param parent parent sha-1 commit hash
     * @param auth author of commit
     * @param msg commit message
     */
    Commit(const std::string& tree, const std::string& parent, const Author& auth, const std::string& msg);

    std::string getType() const override;

    /**
     * git's commit object format is tree <oid>\nparent <oid>\nauthor <info>\ncommitter <info>\n\n<message>\n
     *
     * @see author interface for details on author's effects on the git commit format
     * @return the commits content in git's commit object format
     */
    std::string getContent() const override;

    std::string getTreeOid() const { return treeOid; }
    std::string getParentOid() const { return parentOid; }
    const Author& getAuthor() const { return author; }
    const Author& getCommitter() const { return committer; }
    std::string getMessage() const { return message; }

    std::string toString() const; 
}; 

#endif 
