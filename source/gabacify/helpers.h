#ifndef GABACIFY_HELPERS_H_
#define GABACIFY_HELPERS_H_


#include <cstdint>
#include <string>
#include <vector>

#include "gabac/data_stream.h"


namespace gabacify {


void deriveMinMaxSigned(
        const gabac::DataStream& symbols,
        unsigned int word_size,
        int64_t *min,
        int64_t *max
);


void deriveMinMaxUnsigned(
        const gabac::DataStream& symbols,
        unsigned int word_size,
        uint64_t *min,
        uint64_t *max
);


bool fileExists(
        const std::string& path
);


void generateByteBuffer(
        const gabac::DataStream& symbols,
        unsigned int wordSize,
        gabac::DataStream * const buffer
);


void generateSymbolStream(
        const gabac::DataStream& buffer,
        unsigned int wordSize,
        gabac::DataStream *symbols
);


// based on https://stackoverflow.com/questions/20965960/shannon-entropy
double shannonEntropy(
        const gabac::DataStream& data
);


}  // namespace gabacify


#endif  // GABACIFY_HELPERS_H_
