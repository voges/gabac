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

    // Get the size of the next chunk
    gabac::DataStream sizeBuffer(0, 1);
    sizeBuffer.insert(
            sizeBuffer.end(),
            bytestream.begin() + bytestreamPosition,
            bytestream.begin() + bytestreamPosition + sizeof(uint32_t)
    );
    bytestreamPosition += sizeof(uint32_t);
    gabac::DataStream chunkSizeVector(0, 4);
    generateSymbolStream({sizeBuffer}, 4, &chunkSizeVector);
    uint64_t chunkSize = chunkSizeVector.get(0);

    // Get the next 'chunkSize' bytes from the bytestream
    for (size_t i = 0; i < chunkSize; i++)
    {
        bytes->push_back(bytestream.get(bytestreamPosition++));
    }

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
    gabac::DataStream inverseLutBitstream(0, 1);
    gabac::DataStream inverseLut0bitstream(0, 1);
    gabac::DataStream inverseLut1bitstream(0, 1);
    *bytestreamPosition = extractFromBytestream(bytestream, *bytestreamPosition, &inverseLut0bitstream);
    if(order > 0) {
        *bytestreamPosition = extractFromBytestream(bytestream, *bytestreamPosition, &inverseLut1bitstream);
    }
    GABACIFY_LOG_TRACE << "Read LUT0 bitstream with size: " << inverseLut0bitstream.size();
    GABACIFY_LOG_TRACE << "Read LUT1 bitstream with size: " << inverseLut1bitstream.size();

    size_t lutWordSize = 0;
    if(bits0 <= 8) {
        lutWordSize = 1;
    } else if(bits0 <= 16) {
        lutWordSize = 2;
    } else if(bits0 <= 32) {
        lutWordSize = 4;
    } else if(bits0 <= 64) {
        lutWordSize = 8;
    }

    gabac::DataStream inverseLutTmp = inverseLut0bitstream;
    gabac::decode(
            lutWordSize,
            gabac::BinarizationId::BI,
            {bits0},
            gabac::ContextSelectionId::bypass,
            &inverseLutTmp
    );

    inverseLut->reserve(inverseLutTmp.size());
    for (size_t i = 0; i < inverseLutTmp.size(); ++i)
    {
        uint64_t inverseLutTmpEntry = inverseLutTmp.get(i);
        inverseLut->push_back(static_cast<uint64_t>(inverseLutTmpEntry));
    }

    if (order > 0)
    {
        unsigned bits1 = unsigned(inverseLut->size());

        bits1 = unsigned(std::ceil(std::log2(bits1)));

        size_t lut1WordSize = 0;
        if(bits1 <= 8) {
            lut1WordSize = 1;
        } else if(bits1 <= 16) {
            lut1WordSize = 2;
        } else if(bits1 <= 32) {
            lut1WordSize = 4;
        } else if(bits1 <= 64) {
            lut1WordSize = 8;
        }

        inverseLutTmp = inverseLut1bitstream;
        gabac::decode(
                lut1WordSize,
                gabac::BinarizationId::BI,
                {bits1},
                gabac::ContextSelectionId::bypass,
                &inverseLutTmp
        );

        for (size_t i = 0; i < inverseLutTmp.size(); ++i)
        {
            uint64_t inverseLutTmpEntry = inverseLutTmp.get(i);
            inverseLut1->push_back(static_cast<uint64_t>(inverseLutTmpEntry));
        }
    }
}

//------------------------------------------------------------------------------

static void doDiffCoding(const gabac::DataStream& diffAndLutTransformedSequence,
                         bool enabled,
                         gabac::DataStream *const lutTransformedSequence
){
    // Diff coding
    if (enabled)
    {
        *lutTransformedSequence = diffAndLutTransformedSequence;
        GABACIFY_LOG_TRACE << "Diff coding *en*abled";
        gabac::inverseTransformDiffCoding(lutTransformedSequence);
        return;
    }


    GABACIFY_LOG_TRACE << "Diff coding *dis*abled";
    lutTransformedSequence->reserve(diffAndLutTransformedSequence.size());
    for (size_t i = 0; i < diffAndLutTransformedSequence.size(); ++i)
    {
        uint64_t diffAndLutTransformedSymbol = diffAndLutTransformedSequence.get(i);
        lutTransformedSequence->push_back(static_cast<uint64_t>(diffAndLutTransformedSymbol));
    }
}

//------------------------------------------------------------------------------

static void doLUTCoding(std::vector<gabac::DataStream>& lutSequences,
                        bool enabled,
                        unsigned order,
                        gabac::DataStream *const transformedSequence
){
    if (enabled)
    {
        GABACIFY_LOG_TRACE << "LUT transform *en*abled";

        // Do the inverse LUT transform
        const unsigned LUT_INDEX = 4;
        gabac::transformationInformation[LUT_INDEX].inverseTransform(order, &lutSequences);
        *transformedSequence = lutSequences[0];
        return;
    }

    GABACIFY_LOG_TRACE << "LUT transform *dis*abled";
    *transformedSequence = lutSequences[0]; // TODO: std::move() (currently not possible because of const)
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
        gabac::DataStream *bytestream,
        const Configuration& configuration,
        gabac::DataStream *const sequence
){
    assert(sequence != nullptr);

    sequence->clear();

    // Set up for the inverse sequence transformation
    size_t numTransformedSequences =
            gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].wordsizes.size();

    // Loop through the transformed sequences
    std::vector<gabac::DataStream> transformedSequences;
    size_t bytestreamPosition = 0;
    for (size_t i = 0; i < numTransformedSequences; i++)
    {
        GABACIFY_LOG_TRACE << "Processing transformed sequence: " << i;
        auto transformedSequenceConfiguration = configuration.transformedSequenceConfigurations.at(i);

        gabac::DataStream inverseLut(0, configuration.wordSize);
        gabac::DataStream inverseLut1(0, configuration.wordSize);
        if (transformedSequenceConfiguration.lutTransformationEnabled)
        {
            decodeInverseLUT(
                    *bytestream,
                    &bytestreamPosition,
                    configuration.transformedSequenceConfigurations[i].lutBits,
                    configuration.transformedSequenceConfigurations[i].lutOrder,
                    &inverseLut,
                    &inverseLut1
            );
        }

        gabac::DataStream diffAndLutTransformedSequence(0, configuration.wordSize);

        uint8_t wsize = gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].wordsizes[i];
        if(wsize == 0)
            wsize = configuration.wordSize;

        doEntropyCoding(
                *bytestream,
                configuration.transformedSequenceConfigurations[i],
                wsize,
                &bytestreamPosition,
                &diffAndLutTransformedSequence
        );

        std::vector<gabac::DataStream> lutTransformedSequences(3);
        lutTransformedSequences[0] = gabac::DataStream(0, configuration.wordSize);
        doDiffCoding(
                diffAndLutTransformedSequence,
                configuration.transformedSequenceConfigurations[i].diffCodingEnabled,
                &(lutTransformedSequences[0])
        );
        diffAndLutTransformedSequence.clear();
        diffAndLutTransformedSequence.shrink_to_fit();

        lutTransformedSequences[1] = std::move(inverseLut);
        lutTransformedSequences[2] = std::move(inverseLut1);

        // LUT transform
        gabac::DataStream transformedSequence(0, configuration.wordSize);
        doLUTCoding(
                lutTransformedSequences,
                configuration.transformedSequenceConfigurations[i].lutTransformationEnabled,
                configuration.transformedSequenceConfigurations[i].lutOrder,
                &transformedSequence
        );

        lutTransformedSequences.clear();
        lutTransformedSequences.shrink_to_fit();


        transformedSequences.push_back(std::move(transformedSequence));
    }

    bytestream->clear();
    bytestream->shrink_to_fit();

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

    // Decode with the given configuration
    gabac::DataStream symbols(0, configuration.wordSize);
    decodeWithConfiguration(&bytestream, configuration, &symbols);

    // Write the bytestream
    OutputFile outputFile(outputFilePath);
    outputFile.write(symbols.getData(), 1, symbols.size() * symbols.getWordSize());
    GABACIFY_LOG_INFO << "Wrote buffer of size " << symbols.size() * symbols.getWordSize() << " to: " << outputFilePath;
}

//------------------------------------------------------------------------------

}  // namespace gabacify

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------