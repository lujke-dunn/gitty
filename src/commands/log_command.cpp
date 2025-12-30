#include "../../include/commands/log_command.h"
#include "../../include/core/refs.h"
#include "../../include/core/object_util.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

LogCommand::LogCommand() {}

std::string LogCommand::formatDate(const std::string& authorLine) {
    size_t lastSpace = authorLine.rfind(' ');
    size_t secondLastSpace = authorLine.rfind(' ', lastSpace - 1);

    if (secondLastSpace == std::string::npos) return authorLine;

    std::string timestampStr = authorLine.substr(secondLastSpace + 1, lastSpace - secondLastSpace - 1);
    std::string timezone = authorLine.substr(lastSpace + 1);

    time_t timestamp = std::stol(timestampStr);
    struct tm* timeinfo = localtime(&timestamp);

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%Y", timeinfo);

    return std::string(buffer) + " " + timezone;


}

void LogCommand::printCommit(const std::string& commitSha) {
    std::string commitData = ObjectUtils::readObject(gitPath / "objects", commitSha);
    auto [type, content]   = ObjectUtils::parseObject(commitData);

    if (type != "commit") {
        throw std::runtime_error("not a commit: " + commitSha);
    }

    std::istringstream stream(content);
    std::string line;
    std::string author;
    std::string committer;
    std::string message;

    while (std::getline(stream, line)) {
        if (line.empty()) {
            std::getline(stream, message, '\0');
            break;
        }

        if (line.find("author ") == 0) {
            author = line.substr(7);
        } else if (line.find("committer ") == 0) {
            committer = line.substr(10);
        }
    }

    size_t lastAngleBracket = author.rfind('>');
    std::string authorName = author.substr(0, lastAngleBracket + 1);
    // TODO make this have a pretty yellow color
    std::cout << "commit " << commitSha << std::endl;
    std::cout << "Author: " << authorName << std::endl;
    std::cout << "Date: " << formatDate(author) << std::endl;
    std::cout << std::endl;

    std::istringstream msgStream(message);
    std::string messageLine;
    while (std::getline(msgStream, messageLine)) {
        std::cout << "   " << messageLine << std::endl;
    }
    std::cout << std::endl;
}

int LogCommand::execute() {
    Refs refs(gitPath);
    std::string currentCommit = refs.getHeadCommit();

    if (currentCommit.empty()) {
        std::cerr << "fatal: your branch does not have any commits yet" << std::endl;
        return 1;
    }

    try {
        while (!currentCommit.empty()) {
            printCommit(currentCommit);

            std::string commitData = ObjectUtils::readObject(gitPath / "objects", currentCommit);
            auto [type, content] = ObjectUtils::parseObject(commitData);

            std::istringstream stream(content);
            std::string line;
            std::string parent;

            while (std::getline(stream, line)) {
                if (line.find("parent ") == 0) {
                    parent = line.substr(7);
                    break;
                }
            }

            currentCommit = parent;
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }
}