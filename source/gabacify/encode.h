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
        const gabac::DataStream& bytes,
        gabac::DataStream *bytestream
);

void doDiffTransform(bool enabled,
                     const gabac::DataStream& lutTransformedSequence,
                     gabac::DataStream *diffAndLutTransformedSequence
);

void doLutTransform(bool enabled,
                    const gabac::DataStream& transformedSequence,
                    unsigned int order,
                    gabac::DataStream *const bytestream,
                    std::vector<gabac::DataStream> *const lutSequences,
                    unsigned *bits0
);

void doSequenceTransform(const gabac::DataStream& sequence,
                         const gabac::SequenceTransformationId& transID,
                         uint64_t param,
                         std::vector<gabac::DataStream> *transformedSequences
);


}  // namespace gabacify


#endif  // GABACIFY_ENCODE_H_
