#include "gabacify/encode.h"

#include <cassert>
#include <functional>
#include <iomanip>
#include <limits>
#include <utility>
#include <vector>

#include "gabac/diff_coding.h"
#include "gabac/encoding.h"
#include "gabac/equality_coding.h"
#include "gabac/lut_transform.h"
#include "gabac/match_coding.h"
#include "gabac/rle_coding.h"

#include "gabacify/analysis.h"
#include "gabacify/configuration.h"
#include "gabacify/exceptions.h"
#include "gabacify/helpers.h"
#include "gabacify/input_file.h"
#include "gabacify/log.h"
#include "gabacify/tmp_file.h"


namespace gabacify {


using SequenceTransform = std::function<void(const std::vector<uint64_t>& sequence,
                                             std::vector<std::vector<uint64_t>> *const
)>;

//------------------------------------------------------------------------------

struct TransformationProperties
{
    std::vector<std::string> filepaths; // Paths to output stream tmp files
    std::vector<unsigned int> wordsizes; // Wordsizes of output streams
    uint64_t param; // Transformation parameter
    SequenceTransform transform; // Function to transformation
};

//------------------------------------------------------------------------------

// Appends the size of a stream and the actual bytes to bytestream
static void appendToBytestream(
        const std::vector<unsigned char>& bytes,
        std::vector<unsigned char> *const bytestream
){
    assert(bytestream != nullptr);

    // Append the size of 'bytes' to the bytestream
    std::vector<unsigned char> sizeBuffer;
    generateByteBuffer({static_cast<uint64_t>(bytes.size())}, 4, &sizeBuffer);
    bytestream->insert(bytestream->end(), sizeBuffer.begin(), sizeBuffer.end());

    // Append 'bytes' to the bytestream
    bytestream->insert(bytestream->end(), bytes.begin(), bytes.end());
}

//------------------------------------------------------------------------------


// Recovers transformed stream from file
template<typename T = uint64_t>
static void getTransformedFromFile(const std::vector<std::string>& paths,
                                   const std::vector<unsigned int>& wordSizes,
                                   std::vector<std::vector<T>> *const transformedSequences
){
    for (size_t i = 0; i < paths.size(); ++i)
    {
        InputFile file(paths[i]);
        std::vector<unsigned char> buffer(file.size());
        file.read(&buffer[0], 1, buffer.size());
        transformedSequences->emplace_back();
        generateSymbolStream(buffer, wordSizes[i], &(*transformedSequences).back());
        GABACIFY_LOG_DEBUG << "Transformed sequence "
                           << i
                           << " read with size: "
                           << transformedSequences->back().size();
    }
}

//------------------------------------------------------------------------------


// Saves transformed stream to file
static void writeTransformedToFile(const std::vector<std::string>& paths,
                                   const std::vector<unsigned int>& wordSizes,
                                   const std::vector<std::vector<uint64_t>>& transformedSequences
){
    for (size_t i = 0; i < transformedSequences.size(); ++i)
    {
        TmpFile file(paths[i]);
        std::vector<unsigned char> buffer;
        generateByteBuffer(transformedSequences.at(i), wordSizes[i], &buffer);
        file.write(&buffer[0], 1, buffer.size());
    }

}

//------------------------------------------------------------------------------

// Generates transformed stream using generator FUNC
static void generateTransformed(const std::vector<uint64_t>& sequence,
                                size_t streams,
                                const SequenceTransform& func,
                                std::vector<std::vector<uint64_t>> *const transformedSequences
){
    transformedSequences->resize(streams, std::vector<uint64_t>());
    func(sequence, transformedSequences);
    for (size_t i = 0; i < transformedSequences->size(); ++i)
    {
        GABACIFY_LOG_DEBUG << "Generated transformed sequence "
                           << i
                           << " with size: "
                           << (*transformedSequences)[i].size();
    }

}

//------------------------------------------------------------------------------

static void getTransformed(const std::vector<uint64_t>& sequence,
                           const std::vector<std::string>& paths,
                           const std::vector<unsigned int>& wordSizes,
                           const SequenceTransform& func,
                           std::vector<std::vector<uint64_t>> *const transformedSequences
){
    transformedSequences->clear();

    // Check if files existent
    bool filesExist = true;
    for (const auto& p : paths)
    {
        if (!fileExists(p))
        {
            filesExist = false;
            break;
        }
    }

    // Test if preprocessed files exist
    if (filesExist)
    {
        getTransformedFromFile(paths, wordSizes, transformedSequences);
    }
    else
    {
        generateTransformed(sequence, paths.size(), func, transformedSequences);
        writeTransformedToFile(paths, wordSizes, *transformedSequences);
    }
}

//------------------------------------------------------------------------------

static void createSequenceConf(const std::string& inputFilePath,
                               unsigned int wordSize,
                               const Configuration& configuration,
                               std::vector<TransformationProperties> *const settings
){
    // EqualityCoding
    settings->emplace_back();
    settings->emplace_back();
    settings->back().filepaths = {inputFilePath + ".eq.equalityFlags.tmp", inputFilePath + ".eq.values.tmp"};
    settings->back().wordsizes = {1, wordSize};
    settings->back().param = 0;
    settings->back().transform = [](const std::vector<uint64_t>& sequence,
                                    std::vector<std::vector<uint64_t>> *const transformedSequences
    )
    {
        gabac::transformEqualityCoding(
                sequence,
                &(*transformedSequences)[0],
                &(*transformedSequences)[1]
        );
    };

    // MatchCoding
    settings->emplace_back();
    auto windowSize = static_cast<uint32_t>(configuration.sequenceTransformationParameter);
    settings->back().filepaths = {inputFilePath + ".match." + std::to_string(windowSize) + ".pointers.tmp",
                                  inputFilePath + ".match." + std::to_string(windowSize) + ".lengths.tmp",
                                  inputFilePath + ".match." + std::to_string(windowSize) + ".values.tmp"};
    settings->back().wordsizes = {4, 4, wordSize};
    settings->back().param = windowSize;
    settings->back().transform = [windowSize](const std::vector<uint64_t>& sequence,
                                              std::vector<std::vector<uint64_t>> *const transformedSequences
    )
    {
        gabac::transformMatchCoding(
                sequence,
                windowSize,
                &(*transformedSequences)[0],
                &(*transformedSequences)[1],
                &(*transformedSequences)[2]
        );
    };

    // RleCoding
    settings->emplace_back();
    auto guard = settings->back().param = static_cast<uint64_t>(configuration.sequenceTransformationParameter);
    settings->back().filepaths = {
            inputFilePath + ".rle." + std::to_string(guard) + ".values.tmp",
            inputFilePath + ".rle." + std::to_string(guard) + ".lengths.tmp"};
    settings->back().wordsizes = {wordSize, 4};
    settings->back().transform = [guard](const std::vector<uint64_t>& sequence,
                                         std::vector<std::vector<uint64_t>> *const transformedSequences
    )
    {
        gabac::transformRleCoding(
                sequence,
                guard,
                &(*transformedSequences)[0],
                &(*transformedSequences)[1]
        );
    };

    // LutCoding
    settings->emplace_back();
    settings->back().filepaths = {"", ""};
    settings->back().wordsizes = {wordSize, wordSize};
    settings->back().param = static_cast<uint64_t>(configuration.sequenceTransformationParameter);
    settings->back().transform = [](const std::vector<uint64_t>& sequence,
                                    std::vector<std::vector<uint64_t>> *const lutTransformedSequences
    )
    {
        gabac::transformLutTransform0(
                sequence,
                &(*lutTransformedSequences)[0],
                &(*lutTransformedSequences)[1]
        );
    };
}

//------------------------------------------------------------------------------

static void doSymbolEncoding(const std::vector<uint64_t>& sequence,
                             const Configuration& configuration,
                             const std::vector<TransformationProperties>& settings,
                             std::vector<std::vector<uint64_t>> *const transformedSequences
){
    // Do the sequence transformation
    if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::no_transform)
    {
        GABACIFY_LOG_DEBUG << "Performing sequence transformation 'no_transform'";
        transformedSequences->emplace_back(sequence);
        GABACIFY_LOG_DEBUG << "Generated transformed sequence with size: " << transformedSequences->front().size();
        return;
    }

    GABACIFY_LOG_TRACE << "Encoding sequence of length: " << sequence.size();

    auto id = unsigned(configuration.sequenceTransformationId);
    const std::string names[] = {"no", "equalityCoding", "matchCoding", "rleCoding"};
    GABACIFY_LOG_DEBUG << "Performing sequence transformation " << names[id];
    getTransformed(
            sequence,
            settings[id].filepaths,
            settings[id].wordsizes,
            settings[id].transform,
            transformedSequences
    );
}

//------------------------------------------------------------------------------

static void doLutTransform(const Configuration& configuration,
                           unsigned index,
                           const std::vector<uint64_t>& transformedSequence,
                           const std::vector<TransformationProperties>& conf,
                           std::vector<unsigned char> *const bytestream,
                           std::vector<std::vector<uint64_t >> *const lutSequences
){
    if (!configuration.transformedSequenceConfigurations.at(index).lutTransformationEnabled)
    {
        GABACIFY_LOG_TRACE << "LUT transform *dis*abled";
        lutSequences->emplace_back(transformedSequence);
        return;
    }

    std::string seqTransformExtension = conf[unsigned(configuration.sequenceTransformationId)].filepaths[index];
    GABACIFY_LOG_TRACE << "LUT transform *en*abled";
    GABACIFY_LOG_DEBUG << "used file: " << seqTransformExtension << ".lut.tmp";
    const unsigned LUT_INDEX = 4;
    lutSequences->resize(2);
    getTransformed(
            transformedSequence,
            {seqTransformExtension + ".lutTransformed.tmp",
             seqTransformExtension + ".lut.tmp"},
            conf[LUT_INDEX].wordsizes,
            conf[LUT_INDEX].transform,
            lutSequences
    );


    // GABACIFY_LOG_DEBUG<<"lut size before coding: "<<inverseLutTmp
    auto *data = (int64_t *) (lutSequences->at(1).data());
    std::vector<unsigned char> inverseLutBitstream;
    gabac::encode(
            std::vector<int64_t>(data, data + (*lutSequences)[1].size()),
            gabac::BinarizationId::BI,
            {conf[unsigned(configuration.sequenceTransformationId)].wordsizes[index] * 8},
            gabac::ContextSelectionId::bypass,
            &inverseLutBitstream
    );


    appendToBytestream(inverseLutBitstream, bytestream);
    GABACIFY_LOG_TRACE << "Wrote LUT bitstream with size: " << inverseLutBitstream.size();
}

//------------------------------------------------------------------------------

static void doDiffTransform(bool enabled,
                            const std::vector<uint64_t>& lutTransformedSequence,
                            std::vector<int64_t> *const diffAndLutTransformedSequence
){
    // Diff coding
    if (enabled)
    {
        GABACIFY_LOG_TRACE << "Diff coding *en*abled";
        gabac::transformDiffCoding(lutTransformedSequence, diffAndLutTransformedSequence);
        return;
    }

    diffAndLutTransformedSequence->reserve(lutTransformedSequence.size());

    GABACIFY_LOG_TRACE << "Diff coding *dis*abled";
    for (const auto& lutTransformedSymbol : lutTransformedSequence)
    {
        assert(lutTransformedSymbol <= std::numeric_limits<int64_t>::max());
        diffAndLutTransformedSequence->push_back(static_cast<int64_t>(lutTransformedSymbol));
    }
}

//------------------------------------------------------------------------------

static void encodeStream(const TransformedSequenceConfiguration& conf,
                         const std::vector<int64_t>& diffAndLutTransformedSequence,
                         std::vector<uint8_t> *const bytestream
){
    // Encoding
    std::vector<unsigned char> bitstream;
    gabac::encode(
            diffAndLutTransformedSequence,
            conf.binarizationId,
            conf.binarizationParameters,
            conf.contextSelectionId,
            &bitstream
    );
    GABACIFY_LOG_TRACE << "Bitstream size: " << bitstream.size();
    appendToBytestream(bitstream, bytestream);
}

//------------------------------------------------------------------------------

static void encodeWithConfiguration(
        const std::string& inputFilePath,
        const std::vector<uint64_t>& sequence,
        const Configuration& configuration,
        std::vector<unsigned char> *const bytestream
){

    std::vector<TransformationProperties> settings;
    createSequenceConf(inputFilePath, configuration.wordSize, configuration, &settings);

    std::vector<std::vector<uint64_t>> transformedSequences;
    doSymbolEncoding(sequence, configuration, settings, &transformedSequences);


    // Loop through the transformed sequences
    for (size_t i = 0; i < transformedSequences.size(); i++)
    {
        std::vector<std::vector<uint64_t>> lutTransformedSequences;
        doLutTransform(
                configuration,
                i,
                transformedSequences[i],
                settings,
                bytestream,
                &lutTransformedSequences
        );


        std::vector<int64_t> diffAndLutTransformedSequence;
        doDiffTransform(
                configuration.transformedSequenceConfigurations.at(i).diffCodingEnabled,
                lutTransformedSequences[0],
                &diffAndLutTransformedSequence
        );

        encodeStream(configuration.transformedSequenceConfigurations.at(i), diffAndLutTransformedSequence, bytestream);

    }
}

//------------------------------------------------------------------------------

void encode_plain(const std::string& inputFilePath,
                  const std::string& configurationFilePath,
                  const std::string& outputFilePath
){
    // Read in the entire input file
    InputFile inputFile(inputFilePath);
    std::vector<unsigned char> buffer(inputFile.size());
    inputFile.read(&buffer[0], 1, buffer.size());
    // Read the entire configuration file as a string and convert the JSON
    // input string to the internal GABAC configuration
    InputFile configurationFile(configurationFilePath);
    std::string jsonInput("\0", configurationFile.size());
    configurationFile.read(&jsonInput[0], 1, jsonInput.size());
    Configuration configuration(jsonInput);

    // Generate symbol stream from byte buffer
    std::vector<uint64_t> symbols;
    generateSymbolStream(buffer, configuration.wordSize, &symbols);
    buffer.clear();
    buffer.shrink_to_fit();

    encodeWithConfiguration(inputFilePath, symbols, configuration, &buffer);

    // Write the bytestream
    OutputFile outputFile(outputFilePath);
    outputFile.write(&buffer[0], 1, buffer.size());
    GABACIFY_LOG_INFO << "Wrote bytestream of size " << buffer.size() << " to: " << outputFilePath;
    buffer.clear();
    buffer.shrink_to_fit();
}

//------------------------------------------------------------------------------

void encode_analyze(const std::string& inputFilePath,
                    const std::string& configurationFilePath,
                    const std::string& outputFilePath
){
    // In analysis mode we assume a word size of 1
    unsigned int wordSize = 1;

    InputFile inputFile(inputFilePath);
    std::vector<unsigned char> buffer(inputFile.size());
    inputFile.read(&buffer[0], 1, buffer.size());
    // Generate symbol stream from byte buffer
    std::vector<uint64_t> symbols;
    generateSymbolStream(buffer, wordSize, &symbols);
    buffer.clear();
    buffer.shrink_to_fit();

    // Analyze the input symbols and generate all valid configurations.
    std::vector<Configuration> configurations;
    GABACIFY_LOG_INFO << "Defining configurations ...";
    defineConfigurations(symbols, wordSize, &configurations);

    std::vector<unsigned char> smallestBytestream;
    size_t smallestBytestreamSize = std::numeric_limits<size_t>::max();
    Configuration bestConfiguration;
    size_t configurationCounter = 0;
    GABACIFY_LOG_INFO << "Trying " << configurations.size() << " configurations";
    for (const auto& configuration : configurations)
    {
        std::vector<unsigned char> bytestream;
        encodeWithConfiguration(inputFilePath, symbols, configuration, &bytestream);
        if (bytestream.size() < smallestBytestreamSize)
        {
            smallestBytestream = std::move(bytestream);
            smallestBytestreamSize = smallestBytestream.size();
            bestConfiguration = configuration;
            GABACIFY_LOG_INFO << "Found new best configuration (#"
                              << configurationCounter
                              << "; compressed size: "
                              << smallestBytestream.size()
                              << ")";
            GABACIFY_LOG_INFO << configuration.toPrintableString();
        }
        configurationCounter++;
        if (configurationCounter % 1000 == 0)
        {
            GABACIFY_LOG_INFO << "Progress: " << configurationCounter << " of " << configurations.size();
        }
    }

    // Write the smallest bytestream
    OutputFile outputFile(outputFilePath);
    outputFile.write(&smallestBytestream[0], 1, smallestBytestream.size());
    GABACIFY_LOG_INFO << "Wrote smallest bytestream of size "
                      << smallestBytestream.size()
                      << " to: "
                      << outputFilePath;

    // Write the best configuration as JSON
    std::string jsonString = bestConfiguration.toJsonString();
    OutputFile configurationFile(configurationFilePath);
    configurationFile.write(&jsonString[0], 1, jsonString.size());
    GABACIFY_LOG_INFO << "Wrote best configuration to: " << configurationFilePath;
}

//------------------------------------------------------------------------------

void encode(
        const std::string& inputFilePath,
        bool analyze,
        const std::string& configurationFilePath,
        const std::string& outputFilePath
){
    assert(!inputFilePath.empty());
    assert(!configurationFilePath.empty());
    assert(!outputFilePath.empty());

    if (analyze)
    {
        encode_analyze(inputFilePath, configurationFilePath, outputFilePath);
        return;
    }
    encode_plain(inputFilePath, configurationFilePath, outputFilePath);
}

//------------------------------------------------------------------------------

}  // namespace gabacify
