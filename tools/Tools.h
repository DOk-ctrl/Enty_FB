/* Tools functions */
#pragma once
#include <string>
#include <vector>
class Tools {
  public:
    static std::string getFilenameExt(std::string filename);
    
    // Tools.h Ч добавь перегрузку с €вным ключом
    static void processData(std::vector<char>& data, const std::string& key);
    static void processData(std::vector<unsigned char>& data, const std::string& key);

    static std::vector<char> loadFileToMemory(const std::string& filename);
};

extern const std::string TEXTURE_KEY;
extern const std::string SHADER_KEY;
extern const std::string GLTF_KEY;