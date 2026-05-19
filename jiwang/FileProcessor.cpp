#include "FileProcessor.h"
#include "AESCore.h"
#include "Padding.h"
#include <fstream>
#include <iostream>

bool FileProcessor::readFile(const std::string& path, std::vector<unsigned char>& outData) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    outData.resize(size);
    if (size > 0 && !file.read(reinterpret_cast<char*>(outData.data()), size)) {
        return false;
    }
    return true;
}

bool FileProcessor::writeFile(const std::string& path, const std::vector<unsigned char>& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) return false;
    if (!data.empty() && !file.write(reinterpret_cast<const char*>(data.data()), data.size())) {
        return false;
    }
    return true;
}

bool FileProcessor::encryptFile(const std::string& inputPath, const std::string& outputPath, const unsigned char key[16]) {
    std::vector<unsigned char> plainData;
    if (!readFile(inputPath, plainData)) return false;

    // 添加填充
    Padding::addPadding(plainData);

    // 对每个16字节块加密
    std::vector<unsigned char> cipherData;
    cipherData.reserve(plainData.size());
    unsigned char cipherBlock[16];
    for (size_t i = 0; i < plainData.size(); i += 16) {
        AESEncryptor::encryptBlock(&plainData[i], key, cipherBlock);
        cipherData.insert(cipherData.end(), cipherBlock, cipherBlock + 16);
    }

    return writeFile(outputPath, cipherData);
}

bool FileProcessor::decryptFile(const std::string& inputPath, const std::string& outputPath, const unsigned char key[16]) {
    std::vector<unsigned char> cipherData;
    if (!readFile(inputPath, cipherData)) return false;
    if (cipherData.size() % 16 != 0) return false;

    std::vector<unsigned char> plainData;
    plainData.reserve(cipherData.size());
    unsigned char plainBlock[16];
    for (size_t i = 0; i < cipherData.size(); i += 16) {
        AESDecryptor::decryptBlock(&cipherData[i], key, plainBlock);
        plainData.insert(plainData.end(), plainBlock, plainBlock + 16);
    }

    if (!Padding::removePadding(plainData)) return false;
    return writeFile(outputPath, plainData);
}