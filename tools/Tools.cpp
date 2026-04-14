#include "Tools.h"
#include <fstream>
#include <iostream>

std::string Tools::getFilenameExt(std::string filename) {
  size_t pos = filename.find_last_of('.');
  if (pos != std::string::npos) {
    return filename.substr(pos + 1);
  }
  return std::string();
}

const std::string SECRET_KEY = "GepaPetProject2026!";

void Tools::processData(std::vector<char>& data) {
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= SECRET_KEY[i % SECRET_KEY.length()];
    }
}

void Tools::processData(std::vector<unsigned char>& data) {
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= SECRET_KEY[i % SECRET_KEY.length()];
    }
}

std::vector<char> Tools::loadFileToMemory(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return std::vector<char>();
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        return buffer;
    }
    return std::vector<char>();
}