#include "../../include/core/blob.h"

Blob::Blob(const std::string& data) : data(data) { }

std::string Blob::getType() const {
  return "blob"; 
}

std::string Blob::getContent() const {
  return data;
}

std::string Blob::toString() const {
  return data; 
}

const std::string& Blob::getData() const {
  return data; 
}
