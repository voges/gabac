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

static void decodeInverseLUT(unsigned bits0,
                             unsigned order,
                             gabac::InputStream* inStream,
                             gabac::DataStream *const inverseLut,
                             gabac::DataStream *const inverseLut1
){
    inStream->readStream(inverseLut);

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
        inStream->readStream(inverseLut1);
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

static void doEntropyCoding(const TransformedSequenceConfiguration& transformedSequenceConfiguration,
                            uint8_t wordsize,
                            gabac::InputStream* inStream,
                            gabac::DataStream *const diffAndLutTransformedSequence
){
    inStream->readStream(diffAndLutTransformedSequence);
    GABACIFY_LOG_TRACE << "Bitstream size: " << diffAndLutTransformedSequence->size();

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
        gabac::InputStream *const inStream,
        gabac::OutputStream *const outStream
){

    while(inStream->isValid()) {

        // Set up for the inverse sequence transformation
        size_t numTransformedSequences =
                gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].wordsizes.size();

        // Loop through the transformed sequences
        std::vector<gabac::DataStream> transformedSequences;
        for (size_t i = 0; i < numTransformedSequences; i++) {
            GABACIFY_LOG_TRACE << "Processing transformed sequence: " << i;
            auto transformedSequenceConfiguration = configuration.transformedSequenceConfigurations.at(i);

            std::vector<gabac::DataStream> lutTransformedSequences(3);
            if (transformedSequenceConfiguration.lutTransformationEnabled) {
                decodeInverseLUT(
                        configuration.transformedSequenceConfigurations[i].lutBits,
                        configuration.transformedSequenceConfigurations[i].lutOrder,
                        inStream,
                        &lutTransformedSequences[1],
                        &lutTransformedSequences[2]
                );
            }

            uint8_t wsize =
                    gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].wordsizes[i];
            if (wsize == 0) {
                wsize = configuration.wordSize;
            }

            doEntropyCoding(
                    configuration.transformedSequenceConfigurations[i],
                    wsize,
                    inStream,
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


        gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].inverseTransform(
                configuration.sequenceTransformationParameter,
                &transformedSequences
        );
        GABACIFY_LOG_TRACE << "Decoded sequence of length: " << transformedSequences[0].size();

        outStream->writeBytes(&transformedSequences[0]);
    }
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
    gabac::FileInputStream inStream(inputFile.handle());

    // Read the entire configuration file as a string and convert the JSON
    // input string to the internal GABAC configuration
    InputFile configurationFile(configurationFilePath);
    std::string jsonInput("\0", configurationFile.size());
    configurationFile.read(&jsonInput[0], 1, jsonInput.size());
    Configuration configuration(jsonInput);

    // Write the bytestream
    OutputFile outputFile(outputFilePath);
    gabac::FileOutputStream outStream(outputFile.handle());

    decodeWithConfiguration(configuration, &inStream, &outStream);


    GABACIFY_LOG_INFO << "Wrote buffer of size "
                      << outStream.bytesWritten()
                      << " to: "
                      << outputFilePath;
}

//------------------------------------------------------------------------------

}  // namespace gabacify

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------