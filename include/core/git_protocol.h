#ifndef GIT_PROTOCOL_H
#define GIT_PROTOCOL_H

#include <string>
#include <map>
#include <vector> 

class GitProtocol {
  private: 
    std::string authToken; 
    std::string username;

    std::string httpGet(const std::string& url); 

    std::string httpPost(const std::string& url, const std::string& data, const std::string& contentType); 
  
    std::vector<std::string> parsePktLines(const std::string& data); 

    std::string createPktLine(const std::string& data); 

  public: 
    GitProtocol(); 
    GitProtocol(const std::string& token, const std::string& username); 

    std::map<std::string, std::string> discoverRefs(const std::string& url);

    void sendPack(const std::string& url, 
                  const std::string& refName, 
                  const std::string& oldOid,
                  const std::string& newOid, 
                  const std::string& packData
    ); 
}; 

#endif
