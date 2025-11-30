#ifndef AUTHOR_H
#define AUTHOR_H 

#include <string>
#include <ctime>

class Author {
  private:
    std::string name; 
    std::string email;
    time_t timestamp;

  public:
    // for setting explicit timestamp
    Author(const std::string& authorName, const std::string& authorEmail, time_t authorTimestamp);
    
    // for setting automatic timestamp
    Author(const std::string& authorName, const std::string& authorEmail); 

    std::string getName() const { return name; }
    std::string getEmail() const { return email; }
    time_t getTimestamp() const { return timestamp; }

    std::string toString() const; 
}

#endif
