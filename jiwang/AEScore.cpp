#include "AESCore.h"
#include "AESConstants.h"
#include "KeyExpansion.h"
#include <cstring>

static void subBytes(unsigned char state[4][4]) {
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            state[i][j] = AESConstants::Sbox[state[i][j]];
}

static void invSubBytes(unsigned char state[4][4]) {
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            state[i][j] = AESConstants::InvSbox[state[i][j]];
}

static void shiftRows(unsigned char state[4][4]) {
    unsigned char temp;
    // 第二行左移1字节
    temp = state[1][0];
    for (int i = 0; i < 3; ++i) state[1][i] = state[1][i + 1];
    state[1][3] = temp;
    // 第三行左移2字节
    for (int i = 0; i < 2; ++i) {
        temp = state[2][0];
        for (int j = 0; j < 3; ++j) state[2][j] = state[2][j + 1];
        state[2][3] = temp;
    }
    // 第四行左移3字节
    for (int i = 0; i < 3; ++i) {
        temp = state[3][0];
        for (int j = 0; j < 3; ++j) state[3][j] = state[3][j + 1];
        state[3][3] = temp;
    }
}

static void invShiftRows(unsigned char state[4][4]) {
    unsigned char temp;
    // 第二行右移1字节
    temp = state[1][3];
    for (int i = 3; i > 0; --i) state[1][i] = state[1][i - 1];
    state[1][0] = temp;
    // 第三行右移2字节
    for (int i = 0; i < 2; ++i) {
        temp = state[2][3];
        for (int j = 3; j > 0; --j) state[2][j] = state[2][j - 1];
        state[2][0] = temp;
    }
    // 第四行右移3字节
    for (int i = 0; i < 3; ++i) {
        temp = state[3][3];
        for (int j = 3; j > 0; --j) state[3][j] = state[3][j - 1];
        state[3][0] = temp;
    }
}

static unsigned char galoisMultiply(unsigned char a, unsigned char b) {
    unsigned char p = 0;
    for (int i = 0; i < 8; ++i) {
        if (b & 1) p ^= a;
        bool carry = (a & 0x80);
        a <<= 1;
        if (carry) a ^= 0x1b;
        b >>= 1;
    }
    return p;
}

static void mixColumns(unsigned char state[4][4]) {
    unsigned char temp[4];
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r)
            temp[r] = state[r][c];
        state[0][c] = galoisMultiply(0x02, temp[0]) ^ galoisMultiply(0x03, temp[1]) ^ temp[2] ^ temp[3];
        state[1][c] = temp[0] ^ galoisMultiply(0x02, temp[1]) ^ galoisMultiply(0x03, temp[2]) ^ temp[3];
        state[2][c] = temp[0] ^ temp[1] ^ galoisMultiply(0x02, temp[2]) ^ galoisMultiply(0x03, temp[3]);
        state[3][c] = galoisMultiply(0x03, temp[0]) ^ temp[1] ^ temp[2] ^ galoisMultiply(0x02, temp[3]);
    }
}

static void invMixColumns(unsigned char state[4][4]) {
    unsigned char temp[4];
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r)
            temp[r] = state[r][c];
        state[0][c] = galoisMultiply(0x0e, temp[0]) ^ galoisMultiply(0x0b, temp[1]) ^ galoisMultiply(0x0d, temp[2]) ^ galoisMultiply(0x09, temp[3]);
        state[1][c] = galoisMultiply(0x09, temp[0]) ^ galoisMultiply(0x0e, temp[1]) ^ galoisMultiply(0x0b, temp[2]) ^ galoisMultiply(0x0d, temp[3]);
        state[2][c] = galoisMultiply(0x0d, temp[0]) ^ galoisMultiply(0x09, temp[1]) ^ galoisMultiply(0x0e, temp[2]) ^ galoisMultiply(0x0b, temp[3]);
        state[3][c] = galoisMultiply(0x0b, temp[0]) ^ galoisMultiply(0x0d, temp[1]) ^ galoisMultiply(0x09, temp[2]) ^ galoisMultiply(0x0e, temp[3]);
    }
}

static void addRoundKey(unsigned char state[4][4], const unsigned char* roundKey) {
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            state[i][j] ^= roundKey[i * 4 + j];
}

void AESEncryptor::encryptBlock(const unsigned char plaintext[16], const unsigned char key[16], unsigned char ciphertext[16]) {
    unsigned char state[4][4];
    // 将明文按列填入状态矩阵
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            state[j][i] = plaintext[i * 4 + j];

    KeyExpansion keyExp;
    keyExp.expandKey(key);

    addRoundKey(state, keyExp.getRoundKey(0));

    for (int round = 1; round <= AESConstants::Nr - 1; ++round) {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, keyExp.getRoundKey(round));
    }

    // 最后一轮
    subBytes(state);
    shiftRows(state);
    addRoundKey(state, keyExp.getRoundKey(AESConstants::Nr));

    // 将状态矩阵转换为输出
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            ciphertext[i * 4 + j] = state[j][i];
}

void AESDecryptor::decryptBlock(const unsigned char ciphertext[16], const unsigned char key[16], unsigned char plaintext[16]) {
    unsigned char state[4][4];
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            state[j][i] = ciphertext[i * 4 + j];

    KeyExpansion keyExp;
    keyExp.expandKey(key);

    addRoundKey(state, keyExp.getRoundKey(AESConstants::Nr));

    for (int round = AESConstants::Nr - 1; round >= 1; --round) {
        invShiftRows(state);
        invSubBytes(state);
        addRoundKey(state, keyExp.getRoundKey(round));
        invMixColumns(state);
    }

    invShiftRows(state);
    invSubBytes(state);
    addRoundKey(state, keyExp.getRoundKey(0));

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            plaintext[i * 4 + j] = state[j][i];
}