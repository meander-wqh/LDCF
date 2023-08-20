#ifndef UINT_H_
#define UINT_H_
#include <string>
#include <bitset>



// typedef unsigned char uint8_t;
// typedef unsigned short int uint16_t;
// typedef unsigned long int uint32_t;
// typedef unsigned long long int uint64_t;

std::string uint32ToString(uint32_t number, size_t numBits);
uint32_t StringToUint32(const std::string& binaryStr);

#endif /* UINT_H_ */
