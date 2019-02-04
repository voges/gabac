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
                     gabac::DataStream *diffAndLutTransformedSequence
);

void doLutTransform(bool enabled,
                    unsigned int order,
                    std::vector<gabac::DataStream> *lutSequences,
                    unsigned *bits0
);

void doSequenceTransform(gabac::DataStream& sequence,
                         const gabac::SequenceTransformationId& transID,
                         uint64_t param,
                         std::vector<gabac::DataStream> *transformedSequences
);


}  // namespace gabacify


#endif  // GABACIFY_ENCODE_H_
