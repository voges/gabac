#ifndef TRANSFORMIFY_HELPERS_H_
#define TRANSFORMIFY_HELPERS_H_


#include <cstdint>
#include <string>
#include <vector>


namespace transformify {


bool fileExists(
        const std::string& path
);


void generateByteBuffer(
        const std::vector<uint64_t>& symbols,
        unsigned int wordSize,
        std::vector<unsigned char> * const buffer
);


void generateSymbolStream(
        const std::vector<unsigned char>& buffer,
        unsigned int wordSize,
        std::vector<uint64_t> *symbols
);


}  // namespace transformify


#endif  // TRANSFORMIFY_HELPERS_H_
