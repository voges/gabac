#include "gabacify/decode.h"

#include <cassert>
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
        const std::vector<unsigned char>& bytestream,
        size_t bytestreamPosition,
        std::vector<unsigned char> *const bytes
){
    assert(bytes != nullptr);

    // Set up our output 'bytes'
    bytes->clear();

    // Get the size of the next chunk
    std::vector<unsigned char> sizeBuffer;
    sizeBuffer.insert(
            sizeBuffer.end(),
            bytestream.begin() + bytestreamPosition,
            bytestream.begin() + bytestreamPosition + sizeof(uint32_t)
    );
    bytestreamPosition += sizeof(uint32_t);
    std::vector<uint64_t> chunkSizeVector;
    generateSymbolStream({sizeBuffer}, 4, &chunkSizeVector);
    uint64_t chunkSize = chunkSizeVector.front();

    // Get the next 'chunkSize' bytes from the bytestream
    for (size_t i = 0; i < chunkSize; i++)
    {
        bytes->push_back(bytestream.at(bytestreamPosition++));
    }

    return bytestreamPosition;
}

//------------------------------------------------------------------------------

static void decodeInverseLUT(const std::vector<unsigned char>& bytestream,
                             unsigned wordSize,
                             size_t *const bytestreamPosition,
                             std::vector<uint64_t> *const inverseLut
){
    // Decode the inverse LUT
    std::vector<unsigned char> inverseLutBitstream;
    *bytestreamPosition = extractFromBytestream(bytestream, *bytestreamPosition, &inverseLutBitstream);
    GABACIFY_LOG_TRACE << "Read LUT bitstream with size: " << inverseLutBitstream.size();
    std::vector<int64_t> inverseLutTmp;
    gabac::decode(
            inverseLutBitstream,
            gabac::BinarizationId::BI,
            {wordSize * 8},
            gabac::ContextSelectionId::bypass,
            &inverseLutTmp
    );

    inverseLut->reserve(inverseLutTmp.size());

    for (const auto& inverseLutTmpEntry : inverseLutTmp)
    {
        assert(inverseLutTmpEntry >= 0);
        inverseLut->push_back(static_cast<uint64_t>(inverseLutTmpEntry));
    }
}

//------------------------------------------------------------------------------

static void doDiffCoding(const std::vector<int64_t>& diffAndLutTransformedSequence,
                         bool enabled,
                         std::vector<uint64_t> *const lutTransformedSequence
){
    // Diff coding
    if (enabled)
    {
        GABACIFY_LOG_TRACE << "Diff coding *en*abled";
        gabac::inverseTransformDiffCoding(diffAndLutTransformedSequence, lutTransformedSequence);
        return;
    }


    GABACIFY_LOG_TRACE << "Diff coding *dis*abled";
    lutTransformedSequence->reserve(diffAndLutTransformedSequence.size());
    for (const auto& diffAndLutTransformedSymbol : diffAndLutTransformedSequence)
    {
        assert(diffAndLutTransformedSymbol >= 0);
        lutTransformedSequence->push_back(static_cast<uint64_t>(diffAndLutTransformedSymbol));
    }
}

//------------------------------------------------------------------------------

static void doLUTCoding(const std::vector<std::vector<uint64_t>>& lutSequences,
                        bool enabled,
                        std::vector<uint64_t> *const transformedSequence
){
    if (enabled)
    {
        GABACIFY_LOG_TRACE << "LUT transform *en*abled";

        // Do the inverse LUT transform
        const unsigned LUT_INDEX = 4;
        gabac::transformationInformation[LUT_INDEX].inverseTransform(lutSequences, 0, transformedSequence);
        return;
    }

    GABACIFY_LOG_TRACE << "LUT transform *dis*abled";
    *transformedSequence = lutSequences[0]; // TODO: std::move() (currently not possible because of const)
}

//------------------------------------------------------------------------------

static void doEntropyCoding(const std::vector<unsigned char>& bytestream,
                            const TransformedSequenceConfiguration& transformedSequenceConfiguration,
                            size_t *const bytestreamPosition,
                            std::vector<int64_t> *const diffAndLutTransformedSequence
){
    // Extract encoded diff-and-LUT-transformed sequence (i.e. a
    // bitstream) from the bytestream
    std::vector<unsigned char> bitstream;
    *bytestreamPosition = extractFromBytestream(bytestream, *bytestreamPosition, &bitstream);
    GABACIFY_LOG_TRACE << "Bitstream size: " << bitstream.size();

    // Decoding
    gabac::decode(
            bitstream,
            transformedSequenceConfiguration.binarizationId,
            transformedSequenceConfiguration.binarizationParameters,
            transformedSequenceConfiguration.contextSelectionId,
            diffAndLutTransformedSequence
    );
}

//------------------------------------------------------------------------------

static void decodeWithConfiguration(
        std::vector<unsigned char>* bytestream,
        const Configuration& configuration,
        std::vector<uint64_t> *const sequence
){
    assert(sequence != nullptr);

    sequence->clear();

    // Set up for the inverse sequence transformation
    size_t numTransformedSequences =
            gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].wordsizes.size();

    // Loop through the transformed sequences
    std::vector<std::vector<uint64_t>> transformedSequences;
    size_t bytestreamPosition = 0;
    for (size_t i = 0; i < numTransformedSequences; i++)
    {
        GABACIFY_LOG_TRACE << "Processing transformed sequence: " << i;
        auto transformedSequenceConfiguration = configuration.transformedSequenceConfigurations.at(i);

        unsigned int wordSize =
                gabac::fixWordSizes(
                        gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].wordsizes,
                        configuration.wordSize
                )[i];

        std::vector<uint64_t> inverseLut;
        if (transformedSequenceConfiguration.lutTransformationEnabled)
        {
            decodeInverseLUT(*bytestream, wordSize, &bytestreamPosition, &inverseLut);
        }

        std::vector<int64_t> diffAndLutTransformedSequence;
        doEntropyCoding(
                *bytestream,
                configuration.transformedSequenceConfigurations[i],
                &bytestreamPosition,
                &diffAndLutTransformedSequence
        );

        std::vector<std::vector<uint64_t>> lutTransformedSequences(2);
        doDiffCoding(
                diffAndLutTransformedSequence,
                configuration.transformedSequenceConfigurations[i].diffCodingEnabled,
                &(lutTransformedSequences[0])
        );
        diffAndLutTransformedSequence.clear();
        diffAndLutTransformedSequence.shrink_to_fit();

        lutTransformedSequences[1] = std::move(inverseLut);

        // LUT transform
        std::vector<uint64_t> transformedSequence;
        doLUTCoding(
                lutTransformedSequences,
                configuration.transformedSequenceConfigurations[i].lutTransformationEnabled,
                &transformedSequence
        );

        lutTransformedSequences.clear();
        lutTransformedSequences.shrink_to_fit();


        transformedSequences.push_back(std::move(transformedSequence));
    }

    bytestream->clear();
    bytestream->shrink_to_fit();

    gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].inverseTransform(
            transformedSequences,
            configuration.sequenceTransformationParameter,
            sequence
    );
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
    std::vector<unsigned char> bytestream(bytestreamSize);
    inputFile.read(&bytestream[0], 1, bytestreamSize);

    // Read the entire configuration file as a string and convert the JSON
    // input string to the internal GABAC configuration
    InputFile configurationFile(configurationFilePath);
    std::string jsonInput("\0", configurationFile.size());
    configurationFile.read(&jsonInput[0], 1, jsonInput.size());
    Configuration configuration(jsonInput);

    // Decode with the given configuration
    std::vector<uint64_t> symbols;
    decodeWithConfiguration(&bytestream, configuration, &symbols);

    // Generate byte buffer from symbol stream
    std::vector<unsigned char> buffer;
    generateByteBuffer(symbols, configuration.wordSize, &buffer);
    symbols.clear();
    symbols.shrink_to_fit();

    // Write the bytestream
    OutputFile outputFile(outputFilePath);
    outputFile.write(&buffer[0], 1, buffer.size());
    GABACIFY_LOG_INFO << "Wrote buffer of size " << buffer.size() << " to: " << outputFilePath;
}

//------------------------------------------------------------------------------

}  // namespace gabacify

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------