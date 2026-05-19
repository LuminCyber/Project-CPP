#pragma once
#include <vector>

class Padding {
public:
    // 添加PKCS#7填充，块大小为16字节
    static void addPadding(std::vector<unsigned char>& data);
    // 去除PKCS#7填充，成功返回true，失败返回false
    static bool removePadding(std::vector<unsigned char>& data);
}; 
