#include"uint.h"


std::string uint32ToString(uint32_t number, size_t numBits) {
    std::bitset<32> bits(number);
    std::string binaryStr = bits.to_string();
    return binaryStr.substr(32 - numBits); // 截取有效位数部分
}