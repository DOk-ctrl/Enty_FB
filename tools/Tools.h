/* Tools functions */
#pragma once
#include <string>
#include <vector>
class Tools {
  public:
    static std::string getFilenameExt(std::string filename);

    static void processData(std::vector<char>& data);

    static void processData(std::vector<unsigned char>& data);
    // Вспомогательная функция для чтения файла целиком в вектор
    static std::vector<char> loadFileToMemory(const std::string& filename);
};
