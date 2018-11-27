#include "gabacify/decode.h"

#include <cassert>
#include <vector>

#include "gabac/diff_coding.h"
#include "gabac/decoding.h"
#include "gabac/equality_coding.h"
#include "gabac/lut_transform.h"
#include "gabac/match_coding.h"
#include "gabac/rle_coding.h"

#include "gabacify/configuration.h"
#include "gabacify/exceptions.h"
#include "gabacify/helpers.h"
#include "gabacify/input_file.h"
#include "gabacify/log.h"
#include "gabacify/output_file.h"


namespace gabacify {


static size_t extractFromBytestream(
        const std::vector<unsigned char>& bytestream,
        size_t bytestreamPosition,
        std::vector<unsigned char> * const bytes
){
    assert(bytes != nullptr);

    // Set up our output 'bytes'
    bytes->clear();

    // Get the size of the next chunk
    std::vector<unsigned char> sizeBuffer;
    sizeBuffer.push_back(bytestream.at(bytestreamPosition++));
    sizeBuffer.push_back(bytestream.at(bytestreamPosition++));
    sizeBuffer.push_back(bytestream.at(bytestreamPosition++));
    sizeBuffer.push_back(bytestream.at(bytestreamPosition++));
    std::vector<uint64_t> chunkSizeVector;
    generateSymbolStream({ sizeBuffer }, 4, &chunkSizeVector);
    uint64_t chunkSize = chunkSizeVector.front();

    // Get the next 'chunkSize' bytes from the bytestream
    for (size_t i = 0; i < chunkSize; i++)
    {
        bytes->push_back(bytestream.at(bytestreamPosition++));
    }

    return bytestreamPosition;
}


static void decodeWithConfiguration(
        const std::vector<unsigned char>& bytestream,
        const Configuration& configuration,
        std::vector<uint64_t> * const sequence
){
    assert(sequence != nullptr);

    sequence->clear();

    // Set up for the inverse sequence transformation
    size_t numTransformedSequences = 0;
    if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::no_transform)
    {
        numTransformedSequences = 1;
    }
    else if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::equality_coding)
    {
        numTransformedSequences = 2;
    }
    else if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::match_coding)
    {
        numTransformedSequences = 3;
    }
    else if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::rle_coding)
    {
        numTransformedSequences = 2;
    }
    else
    {
        GABACIFY_DIE("Invalid sequence transformation ID");
    }

    // Loop through the transformed sequences
    std::vector<std::vector<uint64_t>> transformedSequences;
    size_t bytestreamPosition = 0;
    for (size_t i = 0; i < numTransformedSequences; i++)
    {
        GABACIFY_LOG_TRACE << "Processing transformed sequence: " << i;
        auto transformedSequenceConfiguration = configuration.transformedSequenceConfigurations.at(i);
        unsigned int wordSize=configuration.wordSize;
        if(configuration.sequenceTransformationId == gabac::SequenceTransformationId::match_coding)
        {
          if(i!=2)
          {
            wordSize=4;
          }
        }
        else if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::equality_coding)
        {
          if(i==0)
          {
            wordSize=1;
          }
        }
        else if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::rle_coding)
        {
          if(i==1)
          {
            wordSize=4;
          }
        }

        std::vector<uint64_t> inverseLut;
        if (transformedSequenceConfiguration.lutTransformationEnabled)
        {
            // Decode the inverse LUT
            std::vector<unsigned char> inverseLutBitstream;
            bytestreamPosition = extractFromBytestream(bytestream, bytestreamPosition, &inverseLutBitstream);
            GABACIFY_LOG_TRACE << "Read LUT bitstream with size: " << inverseLutBitstream.size();
            std::vector<int64_t> inverseLutTmp;
            gabac::decode(
                inverseLutBitstream,
                gabac::BinarizationId::BI,
                { wordSize * 8 },
                gabac::ContextSelectionId::bypass,
                &inverseLutTmp
            );
            for (const auto& inverseLutTmpEntry : inverseLutTmp)
            {
                assert(inverseLutTmpEntry >= 0);
                inverseLut.push_back(static_cast<uint64_t>(inverseLutTmpEntry));
            }
        }

        // Extract encoded diff-and-LUT-transformed sequence (i.e. a
        // bitstream) from the bytestream
        std::vector<unsigned char> bitstream;
        bytestreamPosition = extractFromBytestream(bytestream, bytestreamPosition, &bitstream);
        GABACIFY_LOG_TRACE << "Bitstream size: " << bitstream.size();

        // Decoding
        std::vector<int64_t> diffAndLutTransformedSequence;
        gabac::decode(
            bitstream,
            transformedSequenceConfiguration.binarizationId,
            transformedSequenceConfiguration.binarizationParameters,
            transformedSequenceConfiguration.contextSelectionId,
            &diffAndLutTransformedSequence
        );

        // Diff coding
        std::vector<uint64_t> lutTransformedSequence;
        if (transformedSequenceConfiguration.diffCodingEnabled)
        {
            GABACIFY_LOG_TRACE << "Diff coding *en*abled";
            gabac::inverseTransformDiffCoding(diffAndLutTransformedSequence, &lutTransformedSequence);
        }
        else
        {
            GABACIFY_LOG_TRACE << "Diff coding *dis*abled";
            for (const auto& diffAndLutTransformedSymbol : diffAndLutTransformedSequence)
            {
                assert(diffAndLutTransformedSymbol >= 0);
                lutTransformedSequence.push_back(static_cast<uint64_t>(diffAndLutTransformedSymbol));
            }
        }

        // LUT transform
        std::vector<uint64_t> transformedSequence;
        if (transformedSequenceConfiguration.lutTransformationEnabled)
        {
            GABACIFY_LOG_TRACE << "LUT transform *en*abled";

            // Do the inverse LUT transform
            gabac::inverseTransformLutTransform0(lutTransformedSequence, inverseLut, &transformedSequence);
        }
        else
        {
            GABACIFY_LOG_TRACE << "LUT transform *dis*abled";
            transformedSequence = std::move(lutTransformedSequence);
        }

        transformedSequences.push_back(std::move(transformedSequence));
    }

    // Do the inverse sequence transformation
    if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::no_transform)
    {
        GABACIFY_LOG_TRACE << "Sequence transformation: no_transform";
        *sequence = std::move(transformedSequences.at(0));
    }
    else if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::equality_coding)
    {
        GABACIFY_LOG_TRACE << "Sequence transformation: equality_coding";
        std::vector<uint64_t> equalityFlags = std::move(transformedSequences.at(0));
        std::vector<uint64_t> values = std::move(transformedSequences.at(1));
        gabac::inverseTransformEqualityCoding(equalityFlags, values, sequence);
    }
    else if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::match_coding)
    {
        GABACIFY_LOG_TRACE << "Sequence transformation: match_coding";
        std::vector<uint64_t> pointers = std::move(transformedSequences.at(0));
        std::vector<uint64_t> lengths = std::move(transformedSequences.at(1));
        std::vector<uint64_t> rawValues = std::move(transformedSequences.at(2));
        gabac::inverseTransformMatchCoding(pointers, lengths, rawValues, sequence);
    }
    else if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::rle_coding)
    {
        GABACIFY_LOG_TRACE << "Sequence transformation: rle_coding";
        auto guard = static_cast<uint64_t>(configuration.sequenceTransformationParameter);
        std::vector<uint64_t> rawValues = std::move(transformedSequences.at(0));
        std::vector<uint64_t> lengths = std::move(transformedSequences.at(1));
        gabac::inverseTransformRleCoding(rawValues, lengths, guard, sequence);
    }
    else
    {
        GABACIFY_DIE("Invalid sequence transformation ID");
    }

    GABACIFY_LOG_TRACE << "Decoded sequence of length: " << sequence->size();
}


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
    decodeWithConfiguration(bytestream, configuration, &symbols);

    // Generate byte buffer from symbol stream
    std::vector<unsigned char> buffer;
    generateByteBuffer(symbols, configuration.wordSize, &buffer);

    // Write the bytestream
    OutputFile outputFile(outputFilePath);
    outputFile.write(&buffer[0], 1, buffer.size());
    GABACIFY_LOG_INFO << "Wrote buffer of size " << buffer.size() << " to: " << outputFilePath;
}


}  // namespace gabacify
