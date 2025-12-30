#include "../../include/core/index.h"
#include <fstream>
#include <sstream> 
#include <iostream>
#include <iomanip>
#include <cstring>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <vector>

Index::Index(const fs::path& gitPath) : indexPath(gitPath / "index") {
  if (fs::exists(indexPath)) {
    load(); 
  }
}

void Index::load() {
  std::ifstream file(indexPath, std::ios::binary);
  if (!file.is_open()) return;

  char signature[4];
  file.read(signature, 4);
  if (std::memcmp(signature, "DIRC", 4) != 0) {
    std::cerr << "Invalid index signature" << std::endl;
    return;
  }

  uint32_t version;
  file.read(reinterpret_cast<char*>(&version), 4);
  version = ntohl(version);

  if (version != 2) {
    std::cerr << "Unsupported index version: " << version << std::endl;
    return;
  }

  uint32_t entryCount;
  file.read(reinterpret_cast<char*>(&entryCount), 4);
  entryCount = ntohl(entryCount);

  for (uint32_t i = 0; i < entryCount; i++) {
    IndexEntry entry;

    file.read(reinterpret_cast<char*>(&entry.ctime_sec), 4);
    entry.ctime_sec = ntohl(entry.ctime_sec);

    file.read(reinterpret_cast<char*>(&entry.ctime_nsec), 4);
    entry.ctime_nsec = ntohl(entry.ctime_nsec);

    file.read(reinterpret_cast<char*>(&entry.mtime_sec), 4);
    entry.mtime_sec  = ntohl(entry.mtime_sec);

    file.read(reinterpret_cast<char*>(&entry.mtime_nsec), 4);
    entry.mtime_nsec = ntohl(entry.mtime_nsec);

    file.read(reinterpret_cast<char*>(&entry.dev), 4);
    entry.dev  = ntohl(entry.dev);

    file.read(reinterpret_cast<char*>(&entry.ino), 4);
    entry.ino = ntohl(entry.ino);

    uint32_t mode;
    file.read(reinterpret_cast<char*>(&mode), 4);
    entry.mode = ntohl(mode);

    file.read(reinterpret_cast<char*>(&entry.uid), 4);
    entry.uid  = ntohl(entry.uid);

    file.read(reinterpret_cast<char*>(&entry.gid), 4);
    entry.gid = ntohl(entry.gid);

    file.read(reinterpret_cast<char*>(&entry.size), 4);
    entry.size = ntohl(entry.size);

    // start reading the sha
    char sha[20];
    file.read(sha, 20);
    std::stringstream ss;
    for (char j : sha) {
      ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(j));
    }
    entry.oid = ss.str();

    uint16_t flags;
    file.read(reinterpret_cast<char*>(&flags), 2);
    flags = ntohs(flags);
    uint16_t pathLen = flags & 0xFFF;

    std::vector<char> pathBuf(pathLen + 1);
    file.read(pathBuf.data(), pathLen);
    pathBuf[pathLen] = '\0';
    entry.path = std::string(pathBuf.data());

    size_t entrySize = 62 + pathLen;
    size_t padding = (8 - (entrySize % 8)) % 8;
    // offset the file pointer by padding
    file.seekg(padding, std::ios::cur);

    entries[entry.path] = entry;
  }
}

void Index::write() {
  std::stringstream indexData;

  indexData.write("DIRC", 4);

  uint32_t version = htonl(2);
  indexData.write(reinterpret_cast<const char*>(&version), 4);

  uint32_t count = htonl(static_cast<uint32_t>(entries.size()));
  indexData.write(reinterpret_cast<const char*>(&count), 4);

  for (const auto& [path, entry] : entries) {
    struct stat st;
    fs::path filePath = indexPath.parent_path().parent_path() / path;
    // TODO this whole thing can be simplified but for now this is fine.
    if (stat(filePath.c_str(), &st) == 0) {
      uint32_t ctime_sec  = htonl(st.st_ctime);
      uint32_t ctime_nsec = htonl(0);
      uint32_t mtime_sec  = htonl(st.st_mtime);
      uint32_t mtime_nsec = htonl(0);
      uint32_t dev        = htonl(st.st_dev);
      uint32_t ino        = htonl(st.st_ino);
      uint32_t mode       = htonl(entry.mode);
      uint32_t uid        = htonl(st.st_uid);
      uint32_t gid        = htonl(st.st_gid);
      uint32_t size       = htonl(st.st_size);

      indexData.write(reinterpret_cast<const char*>(&ctime_sec), 4);
      indexData.write(reinterpret_cast<const char*>(&ctime_nsec), 4);
      indexData.write(reinterpret_cast<const char*>(&mtime_sec), 4);
      indexData.write(reinterpret_cast<const char*>(&mtime_nsec), 4);
      indexData.write(reinterpret_cast<const char*>(&dev), 4);
      indexData.write(reinterpret_cast<const char*>(&ino), 4);
      indexData.write(reinterpret_cast<const char*>(&mode), 4);
      indexData.write(reinterpret_cast<const char*>(&uid), 4);
      indexData.write(reinterpret_cast<const char*>(&gid), 4);
      indexData.write(reinterpret_cast<const char*>(&size), 4);
    } else {
      for (int i = 0; i < 10; i++) {
        uint32_t zero = 0;
        indexData.write(reinterpret_cast<const char*>(&zero), 4);
      }
    }
    for (size_t i = 0; i < entry.oid.length(); i += 2) {
      std::string byteStr = entry.oid.substr(i , 2);
      unsigned char byte = static_cast<unsigned char>(std::stoi(byteStr, nullptr, 16));
      indexData.write(reinterpret_cast<const char*>(&byte), 1);
    }

    // write flags
    uint16_t flags = htons(static_cast<uint16_t>(path.length() & 0xFFF));
    indexData.write(reinterpret_cast<const char*>(&flags), 2);

    indexData.write(path.c_str(), path.length());

    size_t entrySize = 62 + path.length();
    size_t padding   = (8 - (entrySize % 8)) % 8;
    for (size_t i = 0; i < padding; i++) {
      indexData.put('\0');
    }
  }

  std::string content = indexData.str();
  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1(reinterpret_cast<const unsigned char*>(content.c_str()), content.length(), hash);

  std::ofstream file(indexPath, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to write index file");
  }

  file.write(content.c_str(), content.length());
  file.write(reinterpret_cast<const char*>(hash), 20);
  file.close();
}

void Index::add(const std::string& path, const std::string& oid, mode_t mode) {
  IndexEntry entry(path, oid, mode);

  fs::path filePath = indexPath.parent_path().parent_path() / path;
  struct stat st;

  if (stat(filePath.c_str(), &st) == 0) {
    entry.ctime_sec  = st.st_ctime;
    entry.ctime_nsec = 0;
    entry.mtime_sec  = st.st_mtime;
    entry.mtime_nsec = 0;
    entry.dev        = st.st_dev;
    entry.ino        = st.st_ino;
    entry.uid        = st.st_uid;
    entry.gid        = st.st_gid;
    entry.size       = st.st_size;
  }

  entries[path] =  entry;
  write();
}

void Index::remove(const std::string& path) {
  entries.erase(path); 
  write(); 
}

bool Index::isStaged(const std::string& path) const { 
  return entries.find(path) != entries.end();
}

const std::map<std::string, IndexEntry>& Index::getEntries() const {
  return entries;
}

void Index::clear() {
  entries.clear(); 
  write(); 
}
