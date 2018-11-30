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

using InverseSequenceTransform = std::function<void(const std::vector<std::vector<uint64_t>>&,
                                                    std::vector<uint64_t> *const
)>;

//------------------------------------------------------------------------------

struct InverseTransformationProperties
{
    std::vector<unsigned int> wordsizes; // Wordsizes of output streams
    uint64_t param; // Transformation parameter
    InverseSequenceTransform transform; // Function to transformation
};

//------------------------------------------------------------------------------

static void createSequenceConf(unsigned wordSize,
                               uint64_t parameter,
                               std::vector<InverseTransformationProperties> *const out
){

    // No transform
    out->emplace_back();
    out->back().wordsizes = {1};
    out->back().param = 0;
    out->back().transform = [](const std::vector<std::vector<uint64_t>>& transformedSequences,
                               std::vector<uint64_t> *const outputSequence
    )
    {
        *outputSequence = transformedSequences[0];
    };

    // Equality coding
    out->emplace_back();
    out->back().wordsizes = {1, wordSize};
    out->back().param = 0;
    out->back().transform = [](const std::vector<std::vector<uint64_t>>& transformedSequences,
                               std::vector<uint64_t> *const outputSequence
    )
    {
        gabac::inverseTransformEqualityCoding(transformedSequences[0], transformedSequences[1], outputSequence);
    };

    // Match coding
    out->emplace_back();
    out->back().wordsizes = {4, 4, wordSize};
    out->back().param = parameter;
    out->back().transform = [](const std::vector<std::vector<uint64_t>>& transformedSequences,
                               std::vector<uint64_t> *const outputSequence
    )
    {
        gabac::inverseTransformMatchCoding(
                transformedSequences[0],
                transformedSequences[1],
                transformedSequences[2],
                outputSequence
        );
    };

    // RLE Coding
    out->emplace_back();
    out->back().wordsizes = {wordSize, 4};
    out->back().param = parameter;
    out->back().transform = [parameter](const std::vector<std::vector<uint64_t>>& transformedSequences,
                                        std::vector<uint64_t> *const outputSequence
    )
    {
        gabac::inverseTransformRleCoding(transformedSequences[0], transformedSequences[1], parameter, outputSequence);
    };

}

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
    sizeBuffer.push_back(bytestream.at(bytestreamPosition++));
    sizeBuffer.push_back(bytestream.at(bytestreamPosition++));
    sizeBuffer.push_back(bytestream.at(bytestreamPosition++));
    sizeBuffer.push_back(bytestream.at(bytestreamPosition++));
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
    for (const auto& diffAndLutTransformedSymbol : diffAndLutTransformedSequence)
    {
        assert(diffAndLutTransformedSymbol >= 0);
        lutTransformedSequence->push_back(static_cast<uint64_t>(diffAndLutTransformedSymbol));
    }
}

//------------------------------------------------------------------------------

static void doLUTCoding(const std::vector<uint64_t>& lutTransformedSequence,
                        const std::vector<uint64_t>& inverseLut,
                        bool enabled,
                        std::vector<uint64_t> *const transformedSequence
){
    if (enabled)
    {
        GABACIFY_LOG_TRACE << "LUT transform *en*abled";

        // Do the inverse LUT transform
        gabac::inverseTransformLutTransform0(lutTransformedSequence, inverseLut, transformedSequence);
        return;
    }
    else
    {
        GABACIFY_LOG_TRACE << "LUT transform *dis*abled";
        *transformedSequence = std::move(lutTransformedSequence);
    }
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
        const std::vector<unsigned char>& bytestream,
        const Configuration& configuration,
        std::vector<uint64_t> *const sequence
){
    assert(sequence != nullptr);

    sequence->clear();

    std::vector<InverseTransformationProperties> constants;
    createSequenceConf(
            configuration.wordSize,
            static_cast<uint64_t>(configuration.sequenceTransformationParameter),
            &constants
    );
    /*  if (unsigned(configuration.sequenceTransformationId) > unsigned(gabac::SequenceTransformationId::rle_coding)) {
          GABACIFY_DIE("Invalid sequence transformation ID");
      }*/

    // Set up for the inverse sequence transformation
    size_t numTransformedSequences = constants[unsigned(configuration.sequenceTransformationId)].wordsizes.size();

    // Loop through the transformed sequences
    std::vector<std::vector<uint64_t>> transformedSequences;
    size_t bytestreamPosition = 0;
    for (size_t i = 0; i < numTransformedSequences; i++)
    {
        GABACIFY_LOG_TRACE << "Processing transformed sequence: " << i;
        auto transformedSequenceConfiguration = configuration.transformedSequenceConfigurations.at(i);

        unsigned int wordSize = constants[unsigned(configuration.sequenceTransformationId)].wordsizes[i];

        std::vector<uint64_t> inverseLut;
        if (transformedSequenceConfiguration.lutTransformationEnabled)
        {
            decodeInverseLUT(bytestream, wordSize, &bytestreamPosition, &inverseLut);
        }

        std::vector<int64_t> diffAndLutTransformedSequence;
        doEntropyCoding(
                bytestream,
                configuration.transformedSequenceConfigurations[i],
                &bytestreamPosition,
                &diffAndLutTransformedSequence
        );

        std::vector<uint64_t> lutTransformedSequence;
        doDiffCoding(
                diffAndLutTransformedSequence,
                configuration.transformedSequenceConfigurations[i].diffCodingEnabled,
                &lutTransformedSequence
        );

        // LUT transform
        std::vector<uint64_t> transformedSequence;
        doLUTCoding(
                lutTransformedSequence,
                inverseLut,
                configuration.transformedSequenceConfigurations[i].lutTransformationEnabled,
                &transformedSequence
        );

        transformedSequences.push_back(std::move(transformedSequence));
    }

    constants[unsigned(configuration.sequenceTransformationId)].transform(transformedSequences, sequence);
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
    decodeWithConfiguration(bytestream, configuration, &symbols);

    // Generate byte buffer from symbol stream
    std::vector<unsigned char> buffer;
    generateByteBuffer(symbols, configuration.wordSize, &buffer);

    // Write the bytestream
    OutputFile outputFile(outputFilePath);
    outputFile.write(&buffer[0], 1, buffer.size());
    GABACIFY_LOG_INFO << "Wrote buffer of size " << buffer.size() << " to: " << outputFilePath;
}

//------------------------------------------------------------------------------

}  // namespace gabacify

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------