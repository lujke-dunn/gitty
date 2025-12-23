#include "../../include/core/pack.h"
#include "../../include/core/object_util.h"
#include <fstream> 
#include <sstream> 
#include <iomanip> 
#include <zlib.h>
#include <arpa/inet.h>
#include <openssl/sha.h> 
#include <stdexcept> 

PackFile::PackFile(const fs::path& gitPath) : objectsPath(gitPath / "objects") {}

void PackFile::addObject(const std::string& oid) {
  objectOids.push_back(oid); 
}


// Encodes objects size and type in Git's format
// This is used in pack file generation to save space 
void PackFile::writePackedSize(std::ostream& out, size_t size, int type) {
  // First byte encodes both type and first 4 bits of size
  // format: [continue:1][type:3][size:4]
  unsigned char firstByte = ((type & 0x7) << 4) // Type in bits 6-4
                            | ((size & 0xF));   // Size in bits 3-0 
  size >>= 4; // Remove the 4 bits we encoded
  
  if (size > 0) {
    firstByte |= 0x80; // set but 7 (continue bit)
  }

  out.put(firstByte); 
  
  // encode remaining in 7-bit chunks 
  // each byte: [continue:1][size:7]
  while (size > 0) {
    unsigned char byte = (size & 0x7F); 
    size >>= 7; 

    if (size > 0) {
      byte |= 0x80; // set continue bit
    }

    out.put(byte); 
  }
}

std::string PackFile::calculateSHA1(const std::string& data) {
  unsigned char hash[SHA_DIGEST_LENGTH]; 
  SHA1(reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), hash); 

  return std::string(reinterpret_cast<char*>(hash), SHA_DIGEST_LENGTH); 
}

std::string PackFile::generate() {
  std::stringstream packData; 
  packData.write("PACK", 4); 
  uint32_t version = htonl(2); 
  packData.write(reinterpret_cast<const char*>(&version), 4);

  uint32_t count = htonl(static_cast<uint32_t>(objectOids.size()));
  packData.write(reinterpret_cast<const char*>(&count), 4);

  for (const auto& oid : objectOids) {
    std::string objectData = ObjectUtils::readObject(objectsPath, oid); 

    auto [type, content] = ObjectUtils::parseObject(objectData); 

    int typeInt = ObjectUtils::getTypeInt(type); 

    writePackedSize(packData, content.length(), typeInt); 

    uLongf compressedSize = compressBound(content.length());
    std::vector<unsigned char> compressed(compressedSize); 

    int result = compress(
        compressed.data(),
        &compressedSize,
        reinterpret_cast<const unsigned char*>(content.c_str()), 
        content.length()
    );

    if (result != Z_OK) {
      throw std::runtime_error("Failed to compress object: " + oid); 
    }

    packData.write(reinterpret_cast<const char*>(compressed.data()), compressedSize);
  }

  std::string packContent = packData.str(); 
  std::string checksum    = calculateSHA1(packContent); 

  packData.write(checksum.c_str(), 20); 

  return packData.str(); 
}
