#pragma once
#include <string>
#include <vector>

class UserInterface {
public:
    void run();

private:
    void showMainMenu();
    void handleTextEncrypt();
    void handleTextDecrypt();
    void handleFileEncrypt();
    void handleFileDecrypt();
    bool getKeyFromUser(unsigned char key[16]);
    void printHex(const std::vector<unsigned char>& data);
    std::string bytesToHex(const std::vector<unsigned char>& data);
    std::vector<unsigned char> hexToBytes(const std::string& hex);
};