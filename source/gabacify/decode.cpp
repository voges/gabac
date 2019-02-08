#include "gabacify/decode.h"

#include <cassert>
#include <cmath>
#include <vector>

#include "gabac/constants.h"
#include "gabac/diff_coding.h"
#include "gabac/decoding.h"

#include "gabacify/configuration.h"
#include "gabacify/exceptions.h"
#include "gabacify/helpers.h"
#include "gabacify/input_file.h"
#include "gabacify/log.h"
#include "gabacify/output_file.h"


namespace gabacify {


//------------------------------------------------------------------------------

static size_t extractFromBytestream(
        const gabac::DataStream& bytestream,
        size_t bytestreamPosition,
        gabac::DataStream *const bytes
){
    assert(bytes != nullptr);

    // Set up our output 'bytes'
    bytes->clear();

    uint32_t chunkSize = 0;
    auto *ptr = (uint8_t *) &chunkSize;
    for (size_t ctr = 0; ctr < sizeof(uint32_t); ++ctr) {
        *(ptr++) = (uint8_t) bytestream.get(bytestreamPosition++);
    }

    bytes->insert(
            bytes->end(),
            bytestream.begin() + bytestreamPosition,
            bytestream.begin() + bytestreamPosition + chunkSize
    );

    return bytestreamPosition;
}

//------------------------------------------------------------------------------

static void decodeInverseLUT(const gabac::DataStream& bytestream,
                             size_t *const bytestreamPosition,
                             unsigned bits0,
                             unsigned order,
                             gabac::DataStream *const inverseLut,
                             gabac::DataStream *const inverseLut1
){
    // Decode the inverse LUT
    *inverseLut = gabac::DataStream(0, 1);
    *inverseLut1 = gabac::DataStream(0, 1);
    *bytestreamPosition = extractFromBytestream(bytestream, *bytestreamPosition, inverseLut);
    if (order > 0) {
        *bytestreamPosition = extractFromBytestream(bytestream, *bytestreamPosition, inverseLut1);
    }
    GABACIFY_LOG_TRACE << "Read LUT0 bitstream with size: " << inverseLut->size();
    GABACIFY_LOG_TRACE << "Read LUT1 bitstream with size: " << inverseLut1->size();

    size_t lutWordSize = 0;
    if (bits0 <= 8) {
        lutWordSize = 1;
    } else if (bits0 <= 16) {
        lutWordSize = 2;
    } else if (bits0 <= 32) {
        lutWordSize = 4;
    } else if (bits0 <= 64) {
        lutWordSize = 8;
    }

    gabac::decode(
            lutWordSize,
            gabac::BinarizationId::BI,
            {bits0},
            gabac::ContextSelectionId::bypass,
            inverseLut
    );

    if (order > 0) {
        unsigned bits1 = unsigned(inverseLut->size());

        bits1 = unsigned(std::ceil(std::log2(bits1)));

        size_t lut1WordSize = 0;
        if (bits1 <= 8) {
            lut1WordSize = 1;
        } else if (bits1 <= 16) {
            lut1WordSize = 2;
        } else if (bits1 <= 32) {
            lut1WordSize = 4;
        } else if (bits1 <= 64) {
            lut1WordSize = 8;
        }

        gabac::decode(
                lut1WordSize,
                gabac::BinarizationId::BI,
                {bits1},
                gabac::ContextSelectionId::bypass,
                inverseLut1
        );
    }
}

//------------------------------------------------------------------------------

static void doDiffCoding(bool enabled,
                         gabac::DataStream *const lutTransformedSequence
){
    // Diff coding
    if (enabled) {
        GABACIFY_LOG_TRACE << "Diff coding *en*abled";
        gabac::inverseTransformDiffCoding(lutTransformedSequence);
        return;
    }

    GABACIFY_LOG_TRACE << "Diff coding *dis*abled";
}

//------------------------------------------------------------------------------

static void doLUTCoding(bool enabled,
                        unsigned order,
                        std::vector<gabac::DataStream> *const lutSequences
){
    if (enabled) {
        GABACIFY_LOG_TRACE << "LUT transform *en*abled";

        // Do the inverse LUT transform
        const unsigned LUT_INDEX = 4;
        gabac::transformationInformation[LUT_INDEX].inverseTransform(order, lutSequences);
        return;
    }

    GABACIFY_LOG_TRACE << "LUT transform *dis*abled";
}

//------------------------------------------------------------------------------

static void doEntropyCoding(const gabac::DataStream& bytestream,
                            const TransformedSequenceConfiguration& transformedSequenceConfiguration,
                            uint8_t wordsize,
                            size_t *const bytestreamPosition,
                            gabac::DataStream *const diffAndLutTransformedSequence
){
    // Extract encoded diff-and-LUT-transformed sequence (i.e. a
    // bitstream) from the bytestream
    gabac::DataStream bitstream(0, 1);
    *bytestreamPosition = extractFromBytestream(bytestream, *bytestreamPosition, &bitstream);
    GABACIFY_LOG_TRACE << "Bitstream size: " << bitstream.size();

    *diffAndLutTransformedSequence = bitstream;
    // Decoding
    gabac::decode(
            wordsize,
            transformedSequenceConfiguration.binarizationId,
            transformedSequenceConfiguration.binarizationParameters,
            transformedSequenceConfiguration.contextSelectionId,
            diffAndLutTransformedSequence
    );
}

//------------------------------------------------------------------------------

static void decodeWithConfiguration(
        const Configuration& configuration,
        gabac::DataStream *const sequence
){
    assert(sequence != nullptr);

    // Set up for the inverse sequence transformation
    size_t numTransformedSequences =
            gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].wordsizes.size();

    // Loop through the transformed sequences
    std::vector<gabac::DataStream> transformedSequences;
    size_t bytestreamPosition = 0;
    for (size_t i = 0; i < numTransformedSequences; i++) {
        GABACIFY_LOG_TRACE << "Processing transformed sequence: " << i;
        auto transformedSequenceConfiguration = configuration.transformedSequenceConfigurations.at(i);

        std::vector<gabac::DataStream> lutTransformedSequences(3);
        if (transformedSequenceConfiguration.lutTransformationEnabled) {
            decodeInverseLUT(
                    *sequence,
                    &bytestreamPosition,
                    configuration.transformedSequenceConfigurations[i].lutBits,
                    configuration.transformedSequenceConfigurations[i].lutOrder,
                    &lutTransformedSequences[1],
                    &lutTransformedSequences[2]
            );
        }

        uint8_t wsize = gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].wordsizes[i];
        if (wsize == 0) {
            wsize = configuration.wordSize;
        }

        doEntropyCoding(
                *sequence,
                configuration.transformedSequenceConfigurations[i],
                wsize,
                &bytestreamPosition,
                &lutTransformedSequences[0]
        );

        doDiffCoding(
                configuration.transformedSequenceConfigurations[i].diffCodingEnabled,
                &(lutTransformedSequences[0])
        );

        doLUTCoding(
                configuration.transformedSequenceConfigurations[i].lutTransformationEnabled,
                configuration.transformedSequenceConfigurations[i].lutOrder,
                &lutTransformedSequences
        );

        transformedSequences.emplace_back();
        transformedSequences.back().swap(&(lutTransformedSequences[0]));
    }

    sequence->clear();
    sequence->shrink_to_fit();

    gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].inverseTransform(
            configuration.sequenceTransformationParameter,
            &transformedSequences
    );
    sequence->swap(&(transformedSequences[0]));
    GABACIFY_LOG_TRACE << "Decoded sequence of length: " << sequence->size();
}

//------------------------------------------------------------------------------

void decode(
        const std::string& inputFilePath,
        const std::string& configurationFilePath,
        const std::string& outputFilePath
){
    assert(!inputFilePath.empty());
    assert(!configurationFilePath.empty());
    assert(!outputFilePath.empty());

    // Read in the entire input file
    InputFile inputFile(inputFilePath);
    size_t bytestreamSize = inputFile.size();
    gabac::DataStream bytestream(bytestreamSize, 1);
    inputFile.read(bytestream.getData(), 1, bytestream.size());

    // Read the entire configuration file as a string and convert the JSON
    // input string to the internal GABAC configuration
    InputFile configurationFile(configurationFilePath);
    std::string jsonInput("\0", configurationFile.size());
    configurationFile.read(&jsonInput[0], 1, jsonInput.size());
    Configuration configuration(jsonInput);

    decodeWithConfiguration(configuration, &bytestream);

    // Write the bytestream
    OutputFile outputFile(outputFilePath);
    outputFile.write(bytestream.getData(), 1, bytestream.size() * bytestream.getWordSize());
    GABACIFY_LOG_INFO << "Wrote buffer of size "
                      << bytestream.size() * bytestream.getWordSize()
                      << " to: "
                      << outputFilePath;
}

//------------------------------------------------------------------------------

}  // namespace gabacify

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------