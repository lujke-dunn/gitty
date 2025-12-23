#include "../../include/core/git_protocol.h"
#include <curl/curl.h>
#include <sstream> 
#include <iomanip>
#include <stdexcept> 
#include <iostream> 
#include <fstream>

static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
  size_t totalSize = size * nmemb;
  std::string* response = static_cast<std::string*>(userp);
  response->append(static_cast<char*>(contents), totalSize); 
  return totalSize;
}

GitProtocol::GitProtocol() : authToken(""), username("") {}

GitProtocol::GitProtocol(const std::string& token, const std::string& username) : authToken(token), username(username) {}

std::string GitProtocol::httpGet(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    std::string response;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "gitty/1.0");
    
    // Set up headers for Git protocol
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/x-git-receive-pack-advertisement");
    
    if (!authToken.empty()) {
      curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str()); 
      curl_easy_setopt(curl, CURLOPT_PASSWORD, authToken.c_str()); 
    }
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("HTTP GET failed: ") + curl_easy_strerror(res));
    }
    
    return response;
}

std::string GitProtocol::httpPost(const std::string& url,
                                  const std::string& data,
                                  const std::string& contentType) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    std::string response;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "gitty/1.0");
    
    // Set up headers
    struct curl_slist* headers = nullptr;
    std::string contentTypeHeader = "Content-Type: " + contentType;
    headers = curl_slist_append(headers, contentTypeHeader.c_str());
    headers = curl_slist_append(headers, "Accept: application/x-git-receive-pack-result");
    
    if (!authToken.empty()) {
    curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, authToken.c_str());
         }
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    CURLcode res = curl_easy_perform(curl);
    
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode); 

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("HTTP POST failed: ") + curl_easy_strerror(res));
    }
    
    return response;
}

std::vector<std::string> GitProtocol::parsePktLines(const std::string& data) {
  std::vector<std::string> lines; 
  size_t pos = 0; 

  while (pos < data.length()) {
    if (pos + 4 > data.length()) break; 

    std::string lenStr = data.substr(pos, 4); 
    size_t len = std::stoul(lenStr, nullptr, 16); 
    pos += 4; 

    if (len == 0) {
      lines.push_back(""); 
      continue;
    }

    if (len < 4) break; 

    size_t dataLen = len - 4;
    if (pos + dataLen > data.length()) break;

    std::string line = data.substr(pos, dataLen); 
    pos += dataLen; 

    if (!line.empty() && line.back() == '\n') {
      line.pop_back(); 
    }

    lines.push_back(line); 
  }

  return lines; 
}

std::string GitProtocol::createPktLine(const std::string& data) {
  if (data.empty()) {
    return "0000"; // flush packet
  }

  size_t len = 4 + data.length(); 

  std::stringstream ss; 
  ss << std::hex << std::setw(4) << std::setfill('0') << len;
  ss << data;

  return ss.str(); 
}

std::map<std::string, std::string> GitProtocol::discoverRefs(const std::string& url) {
  std::string response = httpGet(url); 
  std::map<std::string, std::string> refs; 

  auto lines = parsePktLines(response); 

  for (const auto& line : lines) {
    if (line.empty()) continue; 

    if (line.find("# service=") == 0) continue;

    // <oid> <refname>\0<capabilities> or <oid> <refname>

    size_t spacePos = line.find(' '); 
    if (spacePos == std::string::npos) continue; 

    std::string oid  = line.substr(0, spacePos); 
    std::string rest = line.substr(spacePos + 1); 

    size_t nullPos = rest.find('\0'); 
    std::string refname = (nullPos != std::string::npos) ? rest.substr(0, nullPos) : rest;

    refs[refname] = oid; 
  }

  return refs; 
}

void GitProtocol::sendPack(const std::string& url, 
                           const std::string& refName,
                           const std::string& oldOid, 
                           const std::string& newOid, 
                           const std::string& packData) {
  std::stringstream request; 

  std::string command = oldOid + " " + newOid + " " + refName;
  command +=  '\0';
  command += " report-status side-band-64k\n"; 
  request  << createPktLine(command); 
  request  << createPktLine(""); 
  request  << packData; 

  std::string requestData = request.str(); 
  
  std::string postUrl = url; 
  size_t pos = postUrl.find("/info/"); 
  if (pos != std::string::npos) {
    postUrl = postUrl.substr(0, pos); 
  }

  postUrl = postUrl + "/git-receive-pack";

  std::string response = httpPost(postUrl, requestData, "application/x-git-receive-pack-request");

  auto lines = parsePktLines(response); 

  for (const auto& line : lines) {
    if (line.empty()) continue; 

    if (line.find("ng ") == 0) {
      throw std::runtime_error("Push rejected: " + line); 
    }
  }

}
