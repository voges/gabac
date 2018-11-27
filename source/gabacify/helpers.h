#ifndef GABACIFY_HELPERS_H_
#define GABACIFY_HELPERS_H_


#include <cstdint>
#include <string>
#include <vector>


namespace gabacify {


void deriveMinMaxSigned(
        const std::vector<int64_t>& symbols,
        unsigned int word_size,
        int64_t *min,
        int64_t *max
);


void deriveMinMaxUnsigned(
        const std::vector<uint64_t>& symbols,
        unsigned int word_size,
        uint64_t *min,
        uint64_t *max
);


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


// based on https://stackoverflow.com/questions/20965960/shannon-entropy
double shannonEntropy(
        const std::vector<uint64_t>& data
);


}  // namespace gabacify


#endif  // GABACIFY_HELPERS_H_
