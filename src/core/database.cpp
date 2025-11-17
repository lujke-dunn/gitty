#include "../../include/core/database.h"
#include <fstream>
#include <vector> 
#include <sstream> 
#include <iomanip> 
#include <openssl/sha.h> 
#include <zlib.h> 

Database::Database(const fs::path& path) : pathname(path) {}

std::string Database::hashContent(const std::string& content) const {
  unsigned char hash[SHA_DIGEST_LENGTH]; 
  SHA1(reinterpret_cast<const unsigned char*>(content.c_str()), content.length(), hash); 

  std::stringstream ss; 
  for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]); 
  }

  return ss.str(); 
}


void Database::writeObject(const std::string& oid, const std::string& content) {
  // Gits object paths typically look something along the lines of 
  // .git/objects/ab/cdef123... 
  fs::path object_path = pathname / oid.substr(0, 2) / oid.substr(2); 

  // the real Git uses zlib compression to compress content 
  uLongf compressed_size = compressBound(content.length()); 
  std::vector<unsigned char> compressed(compressed_size); 

  compress(compressed.data(), &compressed_size, reinterpret_cast<const unsigned char*>(content.c_str()), content.length()); 

  std::ofstream file(object_path, std::ios::binary);
  file.write(reinterpret_cast<const char*>(compressed.data()), compressed_size); 


}

void Database::store(GitObject& object) {
  // Formats: object <size>\0<content>
  std::string content = object.getType() + " " + std::to_string(object.getData().length()) + '\0' + object.getData();

  std::string oid = hashContent(content); 
  object.setOid(oid); 

  writeObject(oid, content); 
}
