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
        std::vector<unsigned char> *const bytestream
);

void doDiffTransform(bool enabled,
                            const std::vector<uint64_t>& lutTransformedSequence,
                            std::vector<int64_t> *const diffAndLutTransformedSequence
);

void doLutTransform(bool enabled,
                           const std::vector<uint64_t>& transformedSequence,
                           unsigned int wordSize,
                           std::vector<unsigned char> *const bytestream,
                           std::vector<std::vector<uint64_t >> *const lutSequences
);

void doSequenceTransform(const std::vector<uint64_t>& sequence,
                                const gabac::SequenceTransformationId& transID,
                                uint64_t param,
                                std::vector<std::vector<uint64_t>> *const transformedSequences
);

using SequenceTransform = std::function<void(const std::vector<uint64_t>& sequence, const uint64_t param,
                                             std::vector<std::vector<uint64_t>> *const
)>;

using SignedBinarizationCheck = std::function<bool(int64_t min, int64_t max, uint64_t parameter
)>;

using UnsignedBinarizationCheck = std::function<bool(uint64_t min, uint64_t max, uint64_t parameter
)>;


struct TransformationProperties
{
    std::string name;
    std::vector<std::string> streamNames;
    std::vector<unsigned int> wordsizes; // Wordsizes of output streams
    SequenceTransform transform; // Function for transformation
};

struct BinarizationProperties
{
    std::string name;
    bool isSigned;
    SignedBinarizationCheck sbCheck;
    UnsignedBinarizationCheck ubCheck;
};

extern const std::vector<TransformationProperties> transformationInformation;
extern const std::vector<BinarizationProperties> binarizationInformation;


}  // namespace gabacify


#endif  // GABACIFY_ENCODE_H_
