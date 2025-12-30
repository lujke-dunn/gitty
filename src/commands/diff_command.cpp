#include "../../include/commands/diff_command.h"
#include "../../include/core/workspace.h"
#include "../../include/core/index.h"
#include "../../include/core/object_util.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <stack>
#include <bits/regex_constants.h>

DiffCommand::DiffCommand() {}

namespace {
    /**
     * Represents a coordinate (oldIndex, newIndex) in the edit graph.
     * - oldIndex: how many lines we've processed from the old file (0 - indexed)
     * - newIndex: how many lines we've processed from the new file (also 0 - indexed) 
     */
    struct MatchExtent {
        int oldIndex;
        int newIndex;
    };

    MatchExtent extentThroughMatches(int startOldIndex, int startNewIndex, const std::vector<std::string>& oldLines, const std::vector<std::string>& newLines) {
        int numOldLines = oldLines.size();
        int numNewLines = newLines.size();

        int oldIndex = startOldIndex;
        int newIndex = startNewIndex;

        while (oldIndex < numOldLines && newIndex < numNewLines && oldLines[oldIndex] == newLines[newIndex]) {
            oldIndex++;
            newIndex++;
        }

        return {oldIndex, newIndex};
    }

    int chooseBetterPath(int diagonal, int editDistance, const std::map<int, int>& furtherReachingX) {
        bool atLeftBoundary = (diagonal == -editDistance);
        bool atRightBoundary = (diagonal == editDistance);

        // Check bounds first, then access with .at() - keys should exist due to bounds check
        if (atLeftBoundary) {
            auto it = furtherReachingX.find(diagonal + 1);
            return (it != furtherReachingX.end()) ? it->second : 0;
        }

        if (atRightBoundary) {
            auto it = furtherReachingX.find(diagonal - 1);
            return (it != furtherReachingX.end()) ? it->second + 1 : 1;
        }

        // Both should exist now
        auto leftIt = furtherReachingX.find(diagonal - 1);
        auto rightIt = furtherReachingX.find(diagonal + 1);

        int leftValue = (leftIt != furtherReachingX.end()) ? leftIt->second : 0;
        int rightValue = (rightIt != furtherReachingX.end()) ? rightIt->second : 0;

        if (leftValue < rightValue) {
            return rightValue;
        } else {
            return leftValue + 1;
        }
    }

    std::vector<std::string> readLines(const fs::path& path) {
        std::vector<std::string> lines;
        std::ifstream file(path);
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }

        return lines;
    }

    std::vector<std::string> readBlobLines(const fs::path& gitPath, const std::string& oid) {
        std::string blobData = ObjectUtils::readObject(gitPath / "objects", oid);
        auto [type, content] = ObjectUtils::parseObject(blobData);

        if (type != "blob") {
            throw std::runtime_error("Expected blob, got " + type);
        }

        std::vector<std::string> lines;
        std::istringstream stream(content);
        std::string line;
        while (std::getline(stream, line)) {
            lines.push_back(line);
        }

        return lines;
    }

    int findPreviousDiagonal(int currentDiagonal, int editDistance, const std::map<int, int>& previousV) {
        bool atLeftBoundary = (currentDiagonal == -editDistance);
        bool atRightBoundary = (currentDiagonal == editDistance);

        if (atLeftBoundary) {
            return currentDiagonal + 1;
        }

        if (atRightBoundary) {
            return currentDiagonal - 1;
        }

        // Check both exist
        auto leftIt = previousV.find(currentDiagonal - 1);
        auto rightIt = previousV.find(currentDiagonal + 1);

        int leftValue = (leftIt != previousV.end()) ? leftIt->second : 0;
        int rightValue = (rightIt != previousV.end()) ? rightIt->second : 0;

        if (leftValue < rightValue) {
            return currentDiagonal + 1;  // Came from above
        } else {
            return currentDiagonal - 1;  // Came from left
        }
    }

}

std::vector<DiffCommand::Hunk> DiffCommand::groupIntoHunks(const std::vector<Edit>& edits, int contextLines) {
    std::vector<Hunk> hunks;
    if (edits.empty()) return hunks;

    Hunk currentHunk;
    currentHunk.oldStart = -1;
    currentHunk.newStart = -1;
    currentHunk.oldCount = 0;
    currentHunk.newCount = 0;

    int consecutiveEquals = 0;

    for (size_t i = 0; i < edits.size(); i++) {
        const auto& edit = edits[i];

        if (edit.type == DiffCommand::Edit::EQUAL) {
            consecutiveEquals++;

            // If we have too many context lines, start a new hunk
            if (consecutiveEquals > 2 * contextLines && !currentHunk.edits.empty()) {
                // Keep last contextLines in current hunk
                for (int j = 0; j < contextLines && !currentHunk.edits.empty(); j++) {
                    currentHunk.edits.pop_back();
                    if (currentHunk.oldCount > 0) currentHunk.oldCount--;
                    if (currentHunk.newCount > 0) currentHunk.newCount--;
                }

                hunks.push_back(currentHunk);

                // Start new hunk
                currentHunk = Hunk();
                currentHunk.oldStart = -1;
                currentHunk.newStart = -1;
                currentHunk.oldCount = 0;
                currentHunk.newCount = 0;
                consecutiveEquals = 0;

                for (int j = contextLines; j > 0 && i - j + 1 < edits.size(); j--) {

                }
            }
        } else {
            consecutiveEquals = 0;
        }

        if (currentHunk.oldStart == -1) {
            currentHunk.oldStart = edit.oldLine + 1;
            currentHunk.newStart = edit.newLine + 1;
        }

        currentHunk.edits.push_back(edit);

        if (edit.type != DiffCommand::Edit::INSERT) {
            currentHunk.oldCount++;
        }
        if (edit.type != DiffCommand::Edit::DELETE) {
            currentHunk.newCount++;
        }
    }

    if (!currentHunk.edits.empty()) {
        hunks.push_back(currentHunk);
    }

    return hunks;
}

void DiffCommand::recordEditOperation(
std::vector<DiffCommand::Edit>& edits,
int& currentOldIndex,
int& currentNewIndex,
int previousOldIndex,
const std::vector<std::string>& oldLines,
const std::vector<std::string>& newLines
) {
    if  (currentOldIndex == previousOldIndex) {
        // vertical edge = insert
        edits.push_back({
            DiffCommand::Edit::INSERT,
            newLines[currentNewIndex -1],
            currentOldIndex,
            currentNewIndex - 1
        });
        currentNewIndex--;
    } else {
        // horizontal edge = delete
        edits.push_back({
            DiffCommand::Edit::DELETE,
            oldLines[currentOldIndex - 1],
            currentOldIndex - 1,
            currentNewIndex
        });
        currentOldIndex--;
    }
}

void DiffCommand::recordMatchingLines(
    std::vector<DiffCommand::Edit>& edits,
    int& currentOldIndex,
    int& currentNewIndex,
    int targetOldIndex,
    int targetNewIndex,
    const std::vector<std::string>& oldLines
    )
{
    while (currentOldIndex > targetOldIndex && currentNewIndex > targetNewIndex) {
        edits.push_back({
            DiffCommand::Edit::EQUAL,
            oldLines[currentOldIndex - 1],
            currentOldIndex - 1,
            currentNewIndex -1
        });
        currentOldIndex--;
        currentNewIndex--;
    }
}


DiffCommand::DiffResult DiffCommand::myersCore(const std::vector<std::string>& oldLines, std::vector<std::string>& newLines) {
    int numOldLines = oldLines.size();
    int numNewLines = newLines.size();
    int maxPossibleEdits = numOldLines + numNewLines;

    std::map<int, int> furthestReachingX;
    std::map<int, std::map<int, int>> editDistanceHistory;

    furthestReachingX[-1] = -1;
    furthestReachingX[1] = 0;

    for (int editDistance = 0; editDistance <= maxPossibleEdits; editDistance++) {
        for (int diagonal = -editDistance; diagonal <= editDistance; diagonal += 2) {

            int oldIndex = chooseBetterPath(diagonal, editDistance, furthestReachingX);
            int newIndex = oldIndex - diagonal;

            auto [finalOldIndex, finalNewIndex] = extentThroughMatches(oldIndex, newIndex, oldLines, newLines);

            furthestReachingX[diagonal] = finalOldIndex;

            if (finalOldIndex >= numOldLines && finalNewIndex >= numNewLines) {
                editDistanceHistory[editDistance] = furthestReachingX;
                return {editDistance, editDistanceHistory};
            }
        }
        editDistanceHistory[editDistance] = furthestReachingX;
    }

    return {maxPossibleEdits, editDistanceHistory};
}

std::vector<DiffCommand::Edit> DiffCommand::myersDiff(const std::vector<std::string>& oldLines, std::vector<std::string>& newLines) {
    auto result = myersCore(oldLines, newLines);
    std::vector<Edit> edits;

    int currentOldIndex = oldLines.size();
    int currentNewIndex = newLines.size();

    for (int editDistance = result.editDistance; editDistance > 0; editDistance--) {
        auto& currentV   = result.editDistanceHistory[editDistance];
        auto& previousV  = result.editDistanceHistory[editDistance - 1];

        int currentDiagonal  = currentOldIndex - currentNewIndex;
        int previousDiagonal = findPreviousDiagonal(currentDiagonal, editDistance, previousV);

        int previousOldIndex = previousV[previousDiagonal];
        int previousNewIndex = previousOldIndex - previousDiagonal;

        recordMatchingLines(edits, currentOldIndex, currentNewIndex, previousOldIndex, previousNewIndex, oldLines);

        if (editDistance > 0) {
            recordEditOperation(edits, currentOldIndex, currentNewIndex, previousOldIndex, oldLines, newLines);
        }
    }

    recordMatchingLines(edits, currentOldIndex, currentNewIndex, 0, 0, oldLines);

    std::reverse(edits.begin(), edits.end());

    return edits;
}

std::vector<DiffCommand::Edit> DiffCommand::characterDiff(
    const std::vector<std::string>& oldStr,
    const std::vector<std::string>& newStr
) {
    if (oldStr.empty() || newStr.empty()) {
        return {};
    }

    std::vector<std::string> oldChars;
    std::vector<std::string> newChars;

    for (char c : oldStr[0]) {
        oldChars.push_back(std::string(1, c));
    }

    for (char c : newStr[0]) {
        newChars.push_back(std::string(1, c));
    }

    return myersDiff(oldChars, newChars);
}

void DiffCommand::printUnifiedDiff(
    const std::vector<Edit>& edits,
    const std::string& indexFileName,
    const std::string& workspaceFileName
) {
    if (edits.empty()) {
        return;
    }

    std::cout << "--- a/" << indexFileName << std::endl;
    std::cout << "+++ b/" << workspaceFileName << std::endl;

    auto hunks = groupIntoHunks(edits, 3);

    for (const auto& hunk : hunks) {
        std::cout << "@@ -" << hunk.oldStart << "," << hunk.oldCount << " +" << hunk.newStart << "," << hunk.newCount << " @@" << std::endl;

        for (const auto& edit : hunk.edits) {
            switch (edit.type) {
                case Edit::EQUAL:
                    std::cout << " " << edit.line << std::endl;
                    break;
                case Edit::DELETE:
                    std::cout << "-" << edit.line << std::endl;
                    break;
                case Edit::INSERT:
                    std::cout << "+" << edit.line << std::endl;
                    break;
            }
        }
    }
}

int DiffCommand::execute() {
    if (positionalArgs.empty()) {
        std::cerr << "usage: diff <file>" << std::endl;
        return 1;
    }

    std::string filepath = positionalArgs[0];

    try {
        Index index(gitPath);
        auto entries = index.getEntries();

        if (entries.find(filepath) == entries.end()) {
            std::cerr << "error: file '" << filepath << "' not in index" << std::endl;
            return 1;
        }

        // Get indexed version (what's committed)
        std::string oid = entries.at(filepath).oid;
        auto indexLines = readBlobLines(gitPath, oid);

        // Get workspace version (current file)
        fs::path workspacePath = rootPath / filepath;
        auto workspaceLines = readLines(workspacePath);

        // Run Myers diff
        auto edits = myersDiff(indexLines, workspaceLines);

        // Print the diff
        printUnifiedDiff(edits, filepath, filepath);

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }
}