#ifndef CONFIG_H
#define CONFIG_H 

#include <string>
#include <map>
#include <fstream> 
#include <sstream>

/**
 * Manages Git configuration in INI format, this can be found in .git/config.
 *
 * Parses .git/config and provides access to repo specific configurations.
 *
 * supports sections, kv-pairs, and comments.
 *
 * @example
 * [core]
 *   repositoryformatversion = 0
 * [user]
 *   name  = Luke Dunn
 *   email = lukedunn011@gmail.com
 *   # email me :)
 *
 * This config is overridden by environment variables and global git config.
 * As I'm writing this, this behaviour does not seem correct; i don't even think it's inline with git spec
 * TODO fix that haha.
 */
class Config {
  private: 
    std::map<std::string, std::map<std::string, std::string>> data;
    std::string configPath;

    std::string trim(const std::string& str);
    void parseLine(const std::string& line, std::string& currentSection); 

  public:
    /**
     * Constructs a config and loads from the specified file.
     * If the file doesn't exist create an empty config.
     *
     * @param path to the config file (.git/config)
     */
    Config(const std::string& path);

    /**
     * Loads configuration from the file.
     * Silently succeeds if file doesn't exist (creates empty).
     */
    void load();

    /**
     * gets config value from .git/config
     *
     * @param section name (e.g. core, or user)
     * @param key name (e.g. name, email, or editor)
     * @return the value as a string or empty string if not found.
     */
    std::string get(const std::string& section, const std::string& key);

    /**
     * sets a config value in memory.
     * does not write to disk until save() is called.
     *
     * @param section name (e.g. core, user, etc)
     * @param key name (e.g. name, email, editor, etc)
     * @param value you want associated with key
     */
    void set(const std::string& section, const std::string& key, const std::string& value);

    /**
     * writes current config to disk, creates or overwrites config file.
     */
    void save();
};

#endif
