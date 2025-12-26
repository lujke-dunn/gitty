#ifndef CHECKOUT_COMMAND_H
#define CHECKOUT_COMMAND_H

#include "command.h"
#include "core/refs.h"
#include "core/entry.h"
#include <map>
#include <string> 

class CheckoutCommand : public Command {
  private:
    bool createBranch; 

    void checkoutBranch(const std::string& branch);
    void checkoutCommit(const std::string& commitSha); 
    void createAndCheckout(const std::string& branch); 

    std::string getCommitTree(const std::string& commitSha);
    std::map<std::string, Entry> readTreeRecursive(const std::string& treeSha, const std::string& prefix = "");
    void updateWorkspace(const std::string& commitSha);
    void updateIndex(const std::string& commitSha);
    bool isValidCommit(const std::string& sha);

  public:
    CheckoutCommand(); 
    int execute() override; 
};

#endif 
