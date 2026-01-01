#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <filesystem>
#include <string>
#include <vector>


namespace fs = std::filesystem;

/**
 * Represents the working directory (workspace) of a Git Repository.
 *
 * Provides file system operations for reading files, listing directories
 * contents, and getting file metadata. Ignoring git internals (.git/)
 * and hidden files (starting with '.').
 */
class Workspace {
  private:
    fs::path pathname;

    /**
     * Recursively lists all regular files under a directory.
     * Ignores .git directory, hidden files, and symlinks.
     * Returns paths relative to workspace root.
     */
    std::vector<fs::path> listFilesRecursively(const fs::path& dir) const;

  public:
    explicit Workspace(const fs::path& path);

    /**
     * Lists all files in workspace, recursively.
     *
     * Scans entire working directory tree, excluding .git and hidden files.
     * returns paths sorted alphabetically (matching git's behaviour).
     *
     * @return Vector of relative file paths as strings
     */
    std::vector<std::string> listFiles() const;

    /**
     * Reads file contents (maybe, if you ask nicely).
     */
    std::string readFile(const std::string& path) const;

    /**
     * Gets file status (type, permissions, etc.).
     */
    fs::file_status statFile(const std::string& path) const;
};

#endif
