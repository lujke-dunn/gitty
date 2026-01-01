#ifndef REMOTE_H
#define REMOTE_H 

#include "config.h"
#include <string> 
#include <filesystem> 

namespace fs = std::filesystem;

/**
 * Represents a remote repository, stores remote configuration (name, URL, fetch refspec) and provides general
 * utilities for protocol detection and service URL generation.
 *
 * @example config format
 *    [remote "origin"]
 *      url = https://github.com/lujke-dunn/gitty.git
 *      fetch = +refs/heads/*:refs/remotes/origin/*
 */
class Remote {
  private: 
    std::string name; // Remote name (e.g. "origin")
    std::string url;  // Remote URL
    std::string fetchSpec;
  
  public:
    Remote(const std::string& remoteName, const std::string& remoteUrl);
    Remote(const std::string& remoteName, const std::string& remoteUrl, const std::string& fetch);

    /**
     * Loads a remote from git config
     * Reads from section: [remote "name"]
     *
     * @param config object to read from
     * @param remoteName of remote load
     * @return remote object
     */
    static Remote fromConfig(Config& config, const std::string& remoteName);

    std::string getName()  const { return name; }
    std::string getUrl()   const { return url; }
    std::string getFetch() const { return fetchSpec; }

    /** @return true if url start with http:// */
    bool isHTTP()  const;

    /** @return true if url starts with https:// */
    bool isHTTPS() const;

    /** @return true if url with ssh:// or git@ */
    bool isSSH()   const;

    /** @return true if url starts with git:// */
    bool isGitProtocol() const; 

    /**
     * Generates Git smart http service URL.
     *
     * Normalises URL (strips .git, trailing slashes) and appends /info/refs?service=<service>
     *
     * @example
     *    Input: https://github.com/lujke-dunn/gitty.git, service="git-receive-pack"
     *    Output: https://github.com/lujke-dunn/gitty.git/info/refs?service=git-recieve-pack
     *
     * @param service git name (e.g., "git-recieve-pack", "git-upload-pack")
     * @return Service URL for HTTP/HTTPS remotes
     */
    std::string getServiceUrl(const std::string& service) const; 
};


#endif 
