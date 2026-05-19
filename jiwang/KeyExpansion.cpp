#include "KeyExpansion.h"
#include "AESConstants.h"
#include <cstring>

void KeyExpansion::expandKey(const unsigned char key[16]) {
    // 第一轮密钥直接拷贝原始密钥
    for (int i = 0; i < 16; ++i)
        roundKeys[0][i] = key[i];

    // 计算后续轮密钥
    for (int i = 1; i <= AESConstants::Nr; ++i) {
        // 前4字节（第0列）由上一轮第3列变换得到
        unsigned int temp;
        memcpy(&temp, &roundKeys[i - 1][12], 4);
        rotWord(temp);
        temp = subWord(temp);
        temp ^= (static_cast<unsigned int>(AESConstants::Rcon[i - 1]) << 24);

        for (int j = 0; j < 4; ++j) {
            roundKeys[i][j] = roundKeys[i - 1][j] ^ ((temp >> (24 - j * 8)) & 0xFF);
        }

        // 剩余3列
        for (int j = 1; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                roundKeys[i][j * 4 + k] = roundKeys[i][(j - 1) * 4 + k] ^ roundKeys[i - 1][j * 4 + k];
            }
        }
    }
}

unsigned char KeyExpansion::subWord(unsigned int word) {
    unsigned char result[4];
    for (int i = 0; i < 4; ++i) {
        unsigned char byte = (word >> (24 - i * 8)) & 0xFF;
        result[i] = AESConstants::Sbox[byte];
    }
    return (result[0] << 24) | (result[1] << 16) | (result[2] << 8) | result[3];
}

void KeyExpansion::rotWord(unsigned int& word) {
    unsigned char bytes[4];
    memcpy(bytes, &word, 4);
    unsigned char temp = bytes[0];
    bytes[0] = bytes[1];
    bytes[1] = bytes[2];
    bytes[2] = bytes[3];
    bytes[3] = temp;
    memcpy(&word, bytes, 4);
}

const unsigned char* KeyExpansion::getRoundKey(int round) const {
    return roundKeys[round].data();
}