#include "Padding.h"

void Padding::addPadding(std::vector<unsigned char>& data) {
    size_t len = data.size();
    size_t paddingLen = 16 - (len % 16);
    for (size_t i = 0; i < paddingLen; ++i) {
        data.push_back(static_cast<unsigned char>(paddingLen));
    }
}

bool Padding::removePadding(std::vector<unsigned char>& data) {
    if (data.empty()) return false;
    size_t lastVal = data.back();
    if (lastVal == 0 || lastVal > 16 || lastVal > data.size()) return false;
    for (size_t i = data.size() - lastVal; i < data.size(); ++i) {
        if (data[i] != lastVal) return false;
    }
    data.resize(data.size() - lastVal);
    return true;
}