#ifndef GIT_PROTOCOL_H
#define GIT_PROTOCOL_H

#include <string>
#include <map>
#include <vector> 

/**
 * Interface for git's http protocol for posting to remote repos.
 *
 * Handles the http based git protocol which is used by remote repo services like gh, and azure.
 * The flow has two distinct steps.
 *  1. discoverRefs() where we query the remotes for the current branch states
 *  2. sendPack() where we send updates and pack data to the remote
 *
 *  In order to pack the data we use pkt-line format where the packets are prefixed by 4 hex digits indicating the packets length.
 *  This also handles the authentication for services like gh via username/token for now...
 *  TODO add more authentication methods.
 */
class GitProtocol {
  private: 
    std::string authToken; // auth token provided by service (e.g. gh personal access token)
    std::string username;  // username for authentication

    /**
     * Performs HTTP GET request with git protocol headers.
     * Includes authentication if token is set.
     *
     * @param url the remotes url
     * @return
     */
    std::string httpGet(const std::string& url);

    /**
     * Performs HTTP POST request with specified content type.
     * Used for sending pack data to remote.
     *
     * @param url
     * @param data
     * @param contentType
     * @return
     */
    std::string httpPost(const std::string& url, const std::string& data, const std::string& contentType);

    /**
     * Parses git pkt-line format into individual lines.
     *
     * pkt-line format: 4-hex digits (packet length) + data.
     * iff packet length is "0000" it is a flush packet (end of section).
     *
     * @param data
     * @return
     */
    std::vector<std::string> parsePktLines(const std::string& data);

    /**
     * Creates a pkt-line formatted packet.
     *
     * @param data packet data
     * @return 4-hex-digit length prefix + data
     */
    std::string createPktLine(const std::string& data); 

  public:
    /** Creates GitProtocol without authentication */
    GitProtocol();

    /**
     * Creates GitProtocol with auth credentials.
     *
     * @param token auth token (e.g. gh PAT)
     * @param username for authentication
     */
    GitProtocol(const std::string& token, const std::string& username);

    /**
     * Discovers remote repository references (branches/tags).
     *
     * Queries the remote's /info/refs endpoint to get current state of all refs.
     * Returning a map of ref names to their current commit oids
     *
     * This query must be made before pushing to know the current commit hash of the target branch,
     * as git's push protocol requires specifying both old and new oids to prevent race conditions and stopping
     * unneccessary pushes.
     *
     * @param url to repository (e.g. https://github.com/lujke-dunn/gitty.git/info/refs?service=git-recieve-pack)
     * @return map of ref names (e.g. "refs/heads/master") to commit oids
     */
    std::map<std::string, std::string> discoverRefs(const std::string& url);

    /**
     * Sends a pack file to update a remote reference
     *
     * Implements Git's recieve-pack protocol to push commits to a remote.
     * sends a command which specifies the old and new oid transition for a ref with the pack data,
     * which contains the objects to transfer.
     *
     * @param url for the repository (just the base url)
     * @param refName is the reference to update
     * @param oldOid the current oid on the remote
     * @param newOid the new proposed oid to push to the remote
     * @param packData the binary pack file containing objects to be pushed
     */
    void sendPack(const std::string& url,
                  const std::string& refName, 
                  const std::string& oldOid,
                  const std::string& newOid, 
                  const std::string& packData
    ); 
}; 

#endif
