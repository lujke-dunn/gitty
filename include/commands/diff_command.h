#ifndef DIFF_COMMAND
#define DIFF_COMMAND

#include "command.h"
#include <string>
#include <vector>

class DiffCommand : public Command {
private:

    /**
     * Represents a single edit operation in a diff.
     *
     * An edit describes one unit's difference between two sequence.
     * A unit can refer to a line difference or a character-level (intra-line) diff.
     *
     * EQUAL means there is no change between sequences.
     * DELETE means the unit was removed from the old version.
     * INSERT means the unit was added in the new version.
     */
    struct Edit {
        enum Type { INSERT, DELETE, EQUAL };
        Type        type;    // Type of edit operation
        std::string line;    // Content of the line
        int         oldLine; // position in old sequence
        int         newLine; // position in new sequence
    };

    struct Hunk {
        int oldStart;
        int oldCount;
        int newStart;
        int newCount;
        std::vector<Edit> edits;
    };

    std::vector<Hunk> groupIntoHunks(const std::vector<Edit>& edits, int contextLines = 3);

    /**
     * Result from Myers diff core algorithm
     *
     * Contains the min-edit distance and the history of the furthest-reaching paths
     * at each edit distance, used for backtracking to reconstruct edits.
     */
    struct DiffResult {
        int editDistance;    // Minimum number of insertion and deletions
        std::map<int, std::map<int, int>> editDistanceHistory; // Furthest x-coordinates for backtracking
    };


    /**
     * Myers O(ND) diff algorithm implementation
     *
     * Finds the shortest edit script length and stores path history.
     *
     * @param oldLines lines from indexed version
     * @param newLines lines from workspace version
     * @return DiffResult
     */
    DiffResult myersCore(const std::vector<std::string>& oldLines, std::vector<std::string>& newLines);

    /**
     * An implementation of Myers difference algorithm.
     *
     * The algorithm computes the minimal diff between two file versions.
     * The algorithm backtracks through the edit graphs to generate the actual sequence of edit operations.
     *
     * @param oldLines refers to the state of a file within the index
     * @param newLines refers to the state of a file within the workspace
     * @return a vector of Edit operations (INSERT, DELETE, EQUALS); allowing us to compare the state of the index vs the workspace
     * and generating a minimal diff between the two.
     * @see http://www.xmailserver.org/diff2.pdf for details on the algorithm.
     */
    std::vector<Edit> myersDiff(const std::vector<std::string> &oldLines, std::vector<std::string> &newLines);

    /**
     * Runs the myers algorithm on individual characters to show precise changes within a line.
     * Used for highlighting intra-line differences.
     * @param oldStr
     * @param newStr
     * @return a vector of character level edits
     */
    std::vector<Edit> characterDiff(const std::vector<std::string>& oldStr, const std::vector<std::string>& newStr);

    /**
     * Prints diff in unified diff format
     *
     * Formats and outputs the edit operations in the standard unified diff.
     *
     * @param edits a vector of edit operations to format
     * @param indexFileName name / path of the indexed file version
     * @param workspaceFileName name / path of the workspace file version
     */
    void printUnifiedDiff(const std::vector<Edit>& edits, const std::string& indexFileName, const std::string& workspaceFileName);

    void recordEditOperation(std::vector<Edit>& edits, int& currentOldIndex, int& currentNewIndex, int previousOldIndex, const std::vector<std::string>& oldLines, const std::vector<std::string>& newLines);

    void recordMatchingLines(std::vector<Edit>& edits, int& currentOldIndex, int& currentNewIndex, int targetOldIndex, int targetNewIndex, const std::vector<std::string>& oldLines);

public:
    DiffCommand();
    int execute() override;

};


#endif