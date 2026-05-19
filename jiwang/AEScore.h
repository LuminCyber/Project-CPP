#pragma once
#include <vector>

class AESEncryptor {
public:
    // 加密一个16字节块
    static void encryptBlock(const unsigned char plaintext[16], const unsigned char key[16], unsigned char ciphertext[16]);
};

class AESDecryptor {
public:
    // 解密一个16字节块
    static void decryptBlock(const unsigned char ciphertext[16], const unsigned char key[16], unsigned char plaintext[16]);
};