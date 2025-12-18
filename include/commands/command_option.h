#ifndef COMMAND_OPTION_H
#define COMMAND_OPTION_H

#include <string>
#include <vector> 

class CommandOption {
  protected: 
    std::string shortFlag;
    std::string longFlag; 
    std::string helpText;

  public: 
    CommandOption(const std::string& shortF, const std::string& longF, const std::string& help);
    virtual ~CommandOption() = default; 

    bool matches(const std::string& arg) const; 

    virtual int parse(int argc, char* argv[], int currentPos) = 0; 

    virtual bool isSet() const = 0; 

    std::string getShortFlag() const { return shortFlag; }
    std::string getLongFlag()  const { return longFlag;  }
    std::string getHelp()      const { return helpText;  }
}; 

class BoolOption : public CommandOption {
  private:
    bool* value; 

  public:
    BoolOption(const std::string& shortF, const std::string& longF, bool* val, const std::string& help);

    int parse(int argc, char* argv[], int currentPos) override; 
    bool isSet() const override; 
}; 

class IntOption : public CommandOption { 
  private: 
    int* value; 
    bool wasSet; 

  public: 
    IntOption(const std::string& shortF, const std::string& longF, int* val, const std::string& help); 

    int parse(int argc, char* argv[], int currentPos) override;
    bool isSet() const override; 
};

class StringOption : public CommandOption {
  private:
    std::string* value;
    bool wasSet; 
  public:
    StringOption(const std::string& shortF, const std::string& longF, std::string* val, const std::string& help); 

    int parse(int argc, char* argv[], int currentPos) override; 
    bool isSet() const override;
};

class MultiStringOption : public CommandOption {
  private: 
    std::vector<std::string>* values;

  public: 
    MultiStringOption(const std::string& shortF, const std::string& longF, 
        std::vector<std::string>* vals, const std::string& help); 

    int parse(int argc, char* argv[], int currentPos) override;
    bool isSet() const override; 
};

#endif
