#include"uint.h"


std::string uint32ToString(uint32_t number, size_t numBits) {
    std::bitset<32> bits(number);
    std::string binaryStr = bits.to_string();
    return binaryStr.substr(32 - numBits); // 截取有效位数部分
}

uint32_t StringToUint32(const std::string& binaryStr){
    uint32_t result = 0;
    for (char c : binaryStr) {
        if (c == '0') {
            result = (result << 1) | 0;
        } else if (c == '1') {
            result = (result << 1) | 1;
        }
    }
    return result;
}