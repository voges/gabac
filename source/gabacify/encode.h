#ifndef GABACIFY_ENCODE_H_
#define GABACIFY_ENCODE_H_

#include <functional>
#include <string>
#include <vector>
#include <gabac/constants.h>

namespace gabacify {


void encode(
        const std::string& inputFilePath,
        bool analyze,
        const std::string& configurationFilePath,
        const std::string& outputFilePath
);

void appendToBytestream(
        const std::vector<unsigned char>& bytes,
        std::vector<unsigned char> *bytestream
);

void doDiffTransform(bool enabled,
                            const std::vector<uint64_t>& lutTransformedSequence,
                            std::vector<int64_t> *diffAndLutTransformedSequence
);

void doLutTransform(bool enabled,
                           const std::vector<uint64_t>& transformedSequence,
                           unsigned int wordSize,
                           std::vector<unsigned char> *bytestream,
                           std::vector<std::vector<uint64_t >> *lutSequences
);

void doSequenceTransform(const std::vector<uint64_t>& sequence,
                                const gabac::SequenceTransformationId& transID,
                                uint64_t param,
                                std::vector<std::vector<uint64_t>> *transformedSequences
);


}  // namespace gabacify


#endif  // GABACIFY_ENCODE_H_
