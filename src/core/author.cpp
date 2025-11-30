#include "../../include/core/author.h"
#include <sstream>
#include <iomanip> 

Author::Author(const std::string& authorName, const std::string& authorEmail, time_t authorTimestamp) :
  name(authorName), email(authorEmail), timestamp(authorTimestamp) {}

Author::Author(const std::string& authorName, const std::string& authorEmail) : name(authorName), email(authorEmail), timestamp(std::time(nullptr)) {}

std::string Author::toString() const {
  std::ostringstream oss; 
  oss << name << " <" << email << "> "; 

  oss << timestamp << " "; 

  char tz_buffer[6]; // Format for +HHMM or -HHMM
  std::tm* timeinfo = std::localtime(&timestamp); 
  std::strftime(tz_buffer, sizeof(tz_buffer), "%z", timeinfo);

  oss << tz_buffer;

  return oss.str(); 
}



