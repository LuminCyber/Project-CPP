#pragma once
#include <array>
#include <cstdint>

class KeyExpansion {
public:
    // 输入16字节原始密钥，生成11个轮密钥（每个16字节），存储在roundKeys[0..10][16]
    void expandKey(const unsigned char key[16]);

    // 获取第round轮的轮密钥（round: 0~10）
    const unsigned char* getRoundKey(int round) const;

private:
    std::array<std::array<unsigned char, 16>, 11> roundKeys;

    unsigned char subWord(unsigned int word);
    void rotWord(unsigned int& word);
};