#include "UserInterface.h"
#include "AESCore.h"
#include "Padding.h"
#include "FileProcessor.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cctype>

void UserInterface::run() {
    while (true) {
        showMainMenu();
        int choice;
        std::cin >> choice;
        std::cin.ignore();
        switch (choice) {
        case 1: handleTextEncrypt(); break;
        case 2: handleTextDecrypt(); break;
        case 3: handleFileEncrypt(); break;
        case 4: handleFileDecrypt(); break;
        case 5: std::cout << "再见！" << std::endl; return;
        default: std::cout << "无效选择，请重新输入。" << std::endl;
        }
    }
}

void UserInterface::showMainMenu() {
    std::cout << "\n========== AES加密交换系统 ==========" << std::endl;
    std::cout << "1. 文本加密" << std::endl;
    std::cout << "2. 文本解密" << std::endl;
    std::cout << "3. 文件加密" << std::endl;
    std::cout << "4. 文件解密" << std::endl;
    std::cout << "5. 退出" << std::endl;
    std::cout << "请选择操作: ";
}

bool UserInterface::getKeyFromUser(unsigned char key[16]) {
    std::string keyStr;
    std::cout << "请输入16字节密钥（32个十六进制字符，如：000102030405060708090a0b0c0d0e0f）: ";
    std::getline(std::cin, keyStr);
    // 去除空格
    keyStr.erase(std::remove_if(keyStr.begin(), keyStr.end(), ::isspace), keyStr.end());
    if (keyStr.length() != 32) {
        std::cout << "错误：密钥必须是32个十六进制字符（16字节）" << std::endl;
        return false;
    }
    for (int i = 0; i < 16; ++i) {
        std::string byteStr = keyStr.substr(i * 2, 2);
        key[i] = static_cast<unsigned char>(std::stoi(byteStr, nullptr, 16));
    }
    return true;
}

void UserInterface::printHex(const std::vector<unsigned char>& data) {
    for (unsigned char c : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    std::cout << std::dec << std::endl;
}

std::string UserInterface::bytesToHex(const std::vector<unsigned char>& data) {
    std::stringstream ss;
    for (unsigned char c : data) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return ss.str();
}

std::vector<unsigned char> UserInterface::hexToBytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteStr = hex.substr(i, 2);
        bytes.push_back(static_cast<unsigned char>(std::stoi(byteStr, nullptr, 16)));
    }
    return bytes;
}

void UserInterface::handleTextEncrypt() {
    std::string plaintext;
    std::cout << "请输入要加密的文本: ";
    std::getline(std::cin, plaintext);

    unsigned char key[16];
    if (!getKeyFromUser(key)) return;

    // 转换为字节并填充
    std::vector<unsigned char> data(plaintext.begin(), plaintext.end());
    Padding::addPadding(data);

    std::vector<unsigned char> ciphertext;
    ciphertext.reserve(data.size());
    unsigned char blockOut[16];
    for (size_t i = 0; i < data.size(); i += 16) {
        AESEncryptor::encryptBlock(&data[i], key, blockOut);
        ciphertext.insert(ciphertext.end(), blockOut, blockOut + 16);
    }

    std::cout << "加密结果（十六进制）: ";
    printHex(ciphertext);
}

void UserInterface::handleTextDecrypt() {
    std::string hexCipher;
    std::cout << "请输入密文（十六进制字符串）: ";
    std::getline(std::cin, hexCipher);
    hexCipher.erase(std::remove_if(hexCipher.begin(), hexCipher.end(), ::isspace), hexCipher.end());

    unsigned char key[16];
    if (!getKeyFromUser(key)) return;

    std::vector<unsigned char> cipherData = hexToBytes(hexCipher);
    if (cipherData.size() % 16 != 0) {
        std::cout << "错误：密文长度必须是16字节的倍数" << std::endl;
        return;
    }

    std::vector<unsigned char> plainData;
    plainData.reserve(cipherData.size());
    unsigned char blockOut[16];
    for (size_t i = 0; i < cipherData.size(); i += 16) {
        AESDecryptor::decryptBlock(&cipherData[i], key, blockOut);
        plainData.insert(plainData.end(), blockOut, blockOut + 16);
    }

    if (!Padding::removePadding(plainData)) {
        std::cout << "错误：解密后填充无效（密钥错误或数据损坏）" << std::endl;
        return;
    }

    std::string plaintext(plainData.begin(), plainData.end());
    std::cout << "解密结果: " << plaintext << std::endl;
}

void UserInterface::handleFileEncrypt() {
    std::string inputPath, outputPath;
    std::cout << "输入源文件路径: ";
    std::getline(std::cin, inputPath);
    std::cout << "输入输出文件路径: ";
    std::getline(std::cin, outputPath);

    unsigned char key[16];
    if (!getKeyFromUser(key)) return;

    if (FileProcessor::encryptFile(inputPath, outputPath, key)) {
        std::cout << "文件加密成功！输出文件: " << outputPath << std::endl;
    }
    else {
        std::cout << "文件加密失败，请检查路径或权限。" << std::endl;
    }
}

void UserInterface::handleFileDecrypt() {
    std::string inputPath, outputPath;
    std::cout << "输入密文文件路径: ";
    std::getline(std::cin, inputPath);
    std::cout << "输入输出文件路径: ";
    std::getline(std::cin, outputPath);

    unsigned char key[16];
    if (!getKeyFromUser(key)) return;

    if (FileProcessor::decryptFile(inputPath, outputPath, key)) {
        std::cout << "文件解密成功！输出文件: " << outputPath << std::endl;
    }
    else {
        std::cout << "文件解密失败，可能是密钥错误或文件损坏。" << std::endl;
    }
}