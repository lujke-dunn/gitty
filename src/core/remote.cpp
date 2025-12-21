#include "../../include/core/remote.h"
#include <stdexcept>
#include <algorithm> 

Remote::Remote(const std::string& remoteName, const std::string& remoteUrl) : name(remoteName), url(remoteUrl), fetchSpec("") {}

Remote::Remote(const std::string& remoteName, const std::string& remoteUrl, const std::string& fetch) 
  : name(remoteName), url(remoteUrl), fetchSpec(fetch) {}

Remote Remote::fromConfig(Config& config, const std::string& remoteName) {
  std::string section = "remote \"" + remoteName + "\"";

  std::string url = config.get(section, "url"); 
  if (url.empty()) {
    throw std::runtime_error("Remote '" + remoteName + "' not found in config"); 
  }

  std::string fetchSpec = config.get(section, "fetch"); 

  return Remote(remoteName, url, fetchSpec); 
}

bool Remote::isHTTP() const {
  return url.find("http://") == 0;
}

bool Remote::isHTTPS() const {
  return url.find("https://") == 0; 
}

bool Remote::isSSH() const {
  return url.find("ssh://") == 0 || url.find("git@") == 0; 
}

bool Remote::isGitProtocol() const {
  return url.find("git://") == 0;  
}

std::string Remote::getServiceUrl(const std::string& service) const {
  if (isHTTP() || isHTTPS()) { 
    std::string baseUrl = url; 
    
    // we remove the .git due to remotes having different formats for e.g. 
    // https://gh.com/user/repo.git or https:://gh.com/user/repo is also a valid format 
    // for simplicity sake we just remove it. 
    if (baseUrl.length() > 4 && baseUrl.substr(baseUrl.length() - 4) == ".git") {
      baseUrl = baseUrl.substr(0, baseUrl.length() - 4); 
    }

    if (!baseUrl.empty() && baseUrl.back() == ('/')) {
      baseUrl = baseUrl.substr(0, baseUrl.length() - 1); 
    }

    return baseUrl + ".git/info/refs?service=" + service; 
  }

  // for now ssh parsing is a can of worms that i am quite afraid of so i'll do that later maybe... 
  throw std::runtime_error("Only http and https remotes allowed sorry"); 

}
