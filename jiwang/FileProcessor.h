#pragma once
#include <string>
#include <vector>

class FileProcessor {
public:
    // 读取整个文件到字节向量
    static bool readFile(const std::string& path, std::vector<unsigned char>& outData);
    // 写入字节向量到文件
    static bool writeFile(const std::string& path, const std::vector<unsigned char>& data);

    // 加密文件（流式处理大文件）
    static bool encryptFile(const std::string& inputPath, const std::string& outputPath, const unsigned char key[16]);
    // 解密文件
    static bool decryptFile(const std::string& inputPath, const std::string& outputPath, const unsigned char key[16]);
}; 
