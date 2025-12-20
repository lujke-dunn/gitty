#ifndef REMOTE_H
#define REMOTE_H 

#include "config.h"
#include <string> 
#include <filesystem> 

namespace fs = std::filesystem;

class Remote {
  private: 
    std::string name;
    std::string url;
    std::string fetchSpec;
  
  public:
    Remote(const std::string& remoteName, const std::string& remoteUrl);
    Remote(const std::string& remoteName, const std::string& remoteUrl, const std::string& fetch); 

    static Remote fromConfig(Config& config, const std::string& remoteName); 

    std::string getName()  const { return name; }
    std::string getUrl()   const { return url; }
    std::string getFetch() const { return fetchSpec; }

    bool isHTTP()  const; 
    bool isHTTPS() const; 
    bool isSSH()   const; 
    bool isGitProtocol() const; 

    std::string getServiceUrl(const std::string& service) const; 
};


#endif 
