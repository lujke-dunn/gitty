#ifndef AUTHOR_H
#define AUTHOR_H 

#include <string>
#include <ctime>

/**
 * This class provides metadata for a git commit.
 *
 * It records the user's name and email which created the commit and the time when the commit is created.
 * In git both the committer and the author use this same format.
 */
class Author {
  private:
    std::string name; 
    std::string email;
    time_t timestamp;


  public:
    /**
     * Creates an author with a specific timestamp.
     * Typically only used when applying a .patch file or when a user passes the flag --date in the commit command.
     * (TODO add support for both patch and --date).
     *
     * @param authorName the author's name.
     * @param authorEmail the author's email.
     * @param authorTimestamp the unix timestamp of when the commit was created.
     */
    Author(const std::string& authorName, const std::string& authorEmail, time_t authorTimestamp);

    /**
     * Creates an author with the current time as the timestamp.
     *
     * @param authorName the author's name.
     * @param authorEmail the author's email.
     */
    Author(const std::string& authorName, const std::string& authorEmail);

    std::string getName() const { return name; }
    std::string getEmail() const { return email; }
    time_t getTimestamp() const { return timestamp; }

    std::string toString() const; 
};

#endif
