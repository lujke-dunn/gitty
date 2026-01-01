#ifndef TREE_BUILDER_H 
#define TREE_BUILDER_H

#include "core/tree.h"
#include "core/entry.h"
#include <vector> 
#include <string>
#include <memory>

/**
 * Builds hierarchical Tree structure from flat list of entries.
 *
 * @example
 *  Converts index entries with paths like "src/death_star_plans/death_star_plan.txt" into nested Tree objects
 *  representing directory structure:
 *      root/
 *          src/
 *          death_star_plans/
 *              death_star_plan.txt
 *
 *  Note: This implies the death star plan was one text file.
 *
 * Used when creating tree objects from staged files during commit.
 */
class TreeBuilder {
  public:
  /**
   * Builds tree hierarchy from flat entry list.
   *
   * Sorts entries and recursively creates nested Tree objects
   * for directory structure. Root tree owns all child trees.
   *
   * @param entries flat list of entries with full paths
   * @return root tree (caller owns pointer, must delete)
   */
  Tree* build(const std::vector<Entry>& entries);

  private:
    /**
    * Extracts parent directory paths.
    * @example "src/death_star_plans/death_star_plan.txt" -> ["src", "src/death_star_plans"]
    */
    std::vector<std::string> getParentDirectories(const std::string& path) const;

    /**
    * Extracts filename from path.
    * @example "src/death_star_plans/death_star_plan.txt" -> "death_star_plan.txt"
    */
    std::string getBasename(const std::string& path) const;

    /**
    * Recursively adds Entry to tree, creating parent trees as needed.
    */
    void addEntry(Tree* tree, const std::vector<std::string>& parents, const Entry& entry);
}; 


#endif
