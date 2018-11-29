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

static std::vector<unsigned char> tempSubseq0;
static gabacify::Configuration configurationTempSubseq0;
static std::vector<unsigned char> tempSubseq0Lut0;
static std::vector<unsigned char> tempSubseq1;
static gabacify::Configuration configurationTempSubseq1;
static std::vector<unsigned char> tempSubseq0Lut1;

using SequenceTransform = std::function<void(const std::vector<uint64_t>& sequence,
                                             std::vector<std::vector<uint64_t>> *const
)>;

//------------------------------------------------------------------------------

static void appendToBytestream(
        const std::vector<unsigned char>& bytes,
        std::vector<unsigned char> *const bytestream
){
    assert(bytestream != nullptr);

    // Append the size of 'bytes' to the bytestream
    std::vector<unsigned char> sizeBuffer;
    generateByteBuffer({static_cast<uint64_t>(bytes.size())}, 4, &sizeBuffer);
    for (const auto& sizeByte : sizeBuffer)
    {
        bytestream->push_back(sizeByte);
    }

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
    std::vector<std::unique_ptr<InputFile>> files;
    for (const auto& p : paths)
    {
        // Freed automatically
        files.push_back(std::unique_ptr<InputFile>(new InputFile(p)));
    }
    for (size_t i = 0; i < files.size(); ++i)
    {
        size_t bufferSize = files[i]->size();
        std::vector<unsigned char> buffer(bufferSize);
        files[i]->read(&buffer[0], 1, bufferSize);
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
    std::vector<std::unique_ptr<TmpFile>> files;
    for (const auto& p : paths)
    {
        // Freed automatically
        files.push_back(std::unique_ptr<TmpFile>(new TmpFile(p)));
    }

    for (size_t i = 0; i < transformedSequences.size(); ++i)
    {
        std::vector<unsigned char> buffer;
        generateByteBuffer(transformedSequences.at(i), wordSizes[i], &buffer);
        files[i]->write(&buffer[0], 1, buffer.size());
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

struct SequenceConf
{
    std::vector<std::string> filepaths;
    std::vector<unsigned int> wordsizes;
    uint64_t param;
    SequenceTransform transform;
};

//------------------------------------------------------------------------------

static void createSequenceConf(const std::string& inputFilePath,
                               unsigned int wordSize,
                               const Configuration& configuration,
                               std::vector<SequenceConf> *const settings
){
    settings->emplace_back();

    // EqualityCoding
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
    auto guard = static_cast<uint64_t>(configuration.sequenceTransformationParameter);
    settings->back().filepaths = {
            inputFilePath + ".rle." + std::to_string(guard) + ".values.tmp",
            inputFilePath + ".rle." + std::to_string(guard) + ".lengths.tmp"};
    settings->back().wordsizes = {wordSize, 4};
    settings->back().param = guard;
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
}

//------------------------------------------------------------------------------

static void doSymbolEncoding(const std::string& inputFilePath,
                             const std::vector<uint64_t>& sequence,
                             const Configuration& configuration,
                             std::vector<std::vector<uint64_t>> *const transformedSequences
){
    assert(bytestream != nullptr);

    std::vector<SequenceConf> settings;
    createSequenceConf(inputFilePath, configuration.wordSize, configuration, &settings);

    GABACIFY_LOG_TRACE << "Encoding sequence of length: " << sequence.size();

    // Do the sequence transformation
    if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::no_transform)
    {
        GABACIFY_LOG_DEBUG << "Performing sequence transformation 'no_transform'";
        transformedSequences->emplace_back(sequence);
        GABACIFY_LOG_DEBUG << "Generated transformed sequence with size: " << transformedSequences->front().size();
        return;
    }
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

static void doLutTransform(const std::string& inputFilePath,
                           const Configuration& configuration,
                           unsigned wordSize,
                           unsigned index,
                           const std::vector<std::vector<uint64_t>>& transformedSequences,
                           std::vector<unsigned char> *const bytestream,
                           std::vector<std::vector<uint64_t >> *const lutSequences
){

    // Extract information
    GABACIFY_LOG_TRACE << "Processing transformed sequence: " << index;
    auto transformedSequence = transformedSequences.at(index);
    auto transformedSequenceConfiguration = configuration.transformedSequenceConfigurations.at(index);
    std::vector<SequenceConf> conf;
    createSequenceConf(inputFilePath, wordSize, configuration, &conf);
    auto id = unsigned(configuration.sequenceTransformationId);
    wordSize = conf[id].wordsizes[index];
    std::string seqTransformExtension = conf[id].filepaths[index];


    std::vector<unsigned char> inverseLutBitstream;
    std::string lutTransformExtension;
    if (transformedSequenceConfiguration.lutTransformationEnabled)
    {
        GABACIFY_LOG_TRACE << "LUT transform *en*abled";
        lutTransformExtension = "." + std::to_string(index) + ".lutTransformed.tmp";
        GABACIFY_LOG_DEBUG << "used file: " << seqTransformExtension << lutTransformExtension;
        getTransformed(
                transformedSequences[index],
                {seqTransformExtension + lutTransformExtension,
                 seqTransformExtension + "." + std::to_string(index) + ".lut.tmp"},
                {wordSize, wordSize},
                [](const std::vector<uint64_t>& sequence,
                   std::vector<std::vector<uint64_t>> *const lutTransformedSequences
                )
                {
                    gabac::transformLutTransform0(
                            sequence,
                            &(*lutTransformedSequences)[0],
                            &(*lutTransformedSequences)[1]
                    );
                },
                lutSequences
        );

        // GABACIFY_LOG_DEBUG<<"lut size before coding: "<<inverseLutTmp
        auto *data = (int64_t *) (lutSequences->at(1).data());
        gabac::encode(
                std::vector<int64_t>(data, data + (*lutSequences)[1].size()),
                gabac::BinarizationId::BI,
                {wordSize * 8},
                gabac::ContextSelectionId::bypass,
                &inverseLutBitstream
        );
        appendToBytestream(inverseLutBitstream, bytestream);
        GABACIFY_LOG_TRACE << "Wrote LUT bitstream with size: " << inverseLutBitstream.size();
        if (index == 0)
        {
            tempSubseq0Lut0 = std::move(inverseLutBitstream);
        }
        else if (index == 1)
        {
            tempSubseq0Lut1 = std::move(inverseLutBitstream);
        }
    }
    else
    {
        GABACIFY_LOG_TRACE << "LUT transform *dis*abled";
        lutSequences->emplace_back(transformedSequences[index]);
    }
}

//------------------------------------------------------------------------------

static void doDiffTransform(const Configuration& configuration,
                            unsigned index,
                            std::vector<std::vector<uint64_t>>& lutTransformedSequences,
                            std::vector<int64_t> *const diffAndLutTransformedSequence
){
    // Diff coding
    auto transformedSequenceConfiguration = configuration.transformedSequenceConfigurations.at(index);
    if (transformedSequenceConfiguration.diffCodingEnabled)
    {
        GABACIFY_LOG_TRACE << "Diff coding *en*abled";
        gabac::transformDiffCoding(lutTransformedSequences[0], diffAndLutTransformedSequence);
    }
    else
    {
        GABACIFY_LOG_TRACE << "Diff coding *dis*abled";
        for (const auto& lutTransformedSymbol : lutTransformedSequences[0])
        {
            assert(lutTransformedSymbol <= std::numeric_limits<int64_t>::max());
            diffAndLutTransformedSequence->push_back(static_cast<int64_t>(lutTransformedSymbol));
        }
    }
}

//------------------------------------------------------------------------------

static void encodeWithConfiguration(
        const std::string& inputFilePath,
        const std::vector<uint64_t>& sequence,
        const Configuration& configuration,
        std::vector<unsigned char> *const bytestream
){

    std::vector<std::vector<uint64_t>> transformedSequences;
    doSymbolEncoding(inputFilePath, sequence, configuration, &transformedSequences);

    unsigned wordSize = configuration.wordSize;

    // Loop through the transformed sequences
    for (size_t i = 0; i < transformedSequences.size(); i++)
    {
        if (i == 0)
        {
            if (configuration.equal(configurationTempSubseq0, i))
            {
                GABACIFY_LOG_DEBUG << "reuse of config";
                if (configuration.transformedSequenceConfigurations.at(i).lutTransformationEnabled)
                {
                    appendToBytestream(tempSubseq0Lut0, bytestream);
                }
                appendToBytestream(tempSubseq0, bytestream);
                continue;
            }
        }
        else if (i == 1)
        {
            //TODO: i wanted something like config1.equalTo(config2, i)
            if (configuration.equal(configurationTempSubseq1, i))
            {
                GABACIFY_LOG_DEBUG << "reuse of config";
                if (configuration.transformedSequenceConfigurations.at(i).lutTransformationEnabled)
                {
                    appendToBytestream(tempSubseq0Lut1, bytestream);
                }
                appendToBytestream(tempSubseq1, bytestream);
                continue;
            }
        }
        else
        {
            // For i == 2 (which only occcurs using match_coding) the
            // configuration will always change. Thus a re-use does not make
            // sense.
        }

        std::vector<std::vector<uint64_t>> lutTransformedSequences;
        doLutTransform(
                inputFilePath,
                configuration,
                wordSize,
                i,
                transformedSequences,
                bytestream,
                &lutTransformedSequences
        );

        std::vector<int64_t> diffAndLutTransformedSequence;
        doDiffTransform(configuration, i, lutTransformedSequences, &diffAndLutTransformedSequence);

        // Encoding
        auto transformedSequenceConfiguration = configuration.transformedSequenceConfigurations.at(i);
        std::vector<unsigned char> bitstream;
        gabac::encode(
                diffAndLutTransformedSequence,
                transformedSequenceConfiguration.binarizationId,
                transformedSequenceConfiguration.binarizationParameters,
                transformedSequenceConfiguration.contextSelectionId,
                &bitstream
        );
        GABACIFY_LOG_TRACE << "Bitstream size: " << bitstream.size();
        appendToBytestream(bitstream, bytestream);
        if (i == 0)
        {
            tempSubseq0 = std::move(bitstream);
            configurationTempSubseq0 = configuration;
        }
        else if (i == 1)
        {
            tempSubseq1 = std::move(bitstream);
            configurationTempSubseq1 = configuration;
        }
    }
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

    // Read in the entire input file
    InputFile inputFile(inputFilePath);
    size_t bufferSize = inputFile.size();
    std::vector<unsigned char> buffer(bufferSize);
    inputFile.read(&buffer[0], 1, bufferSize);

    if (analyze)
    {
        // In analysis mode we assume a word size of 1
        unsigned int wordSize = 1;

        // Generate symbol stream from byte buffer
        std::vector<uint64_t> symbols;
        generateSymbolStream(buffer, wordSize, &symbols);
        buffer.clear();
        buffer.shrink_to_fit();
        // if (symbols.size()>2*1024*1024)
        //   symbols.erase(symbols.begin()+2*1024*1024,symbols.end());

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
        TmpFile outputFile(outputFilePath);
        outputFile.write(&smallestBytestream[0], 1, smallestBytestream.size());
        GABACIFY_LOG_INFO << "Wrote smallest bytestream of size "
                          << smallestBytestream.size()
                          << " to: "
                          << outputFilePath;

        // Write the best configuration as JSON
        std::string jsonString = bestConfiguration.toJsonString();
        TmpFile configurationFile(configurationFilePath);
        configurationFile.write(&jsonString[0], 1, jsonString.size());
        GABACIFY_LOG_INFO << "Wrote best configuration to: " << configurationFilePath;
    }
    else
    {
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

        // Encode with the given configuration
        std::vector<unsigned char> bytestream;
        encodeWithConfiguration(inputFilePath, symbols, configuration, &bytestream);

        // Write the bytestream
        TmpFile outputFile(outputFilePath);
        outputFile.write(&bytestream[0], 1, bytestream.size());
        GABACIFY_LOG_INFO << "Wrote bytestream of size " << bytestream.size() << " to: " << outputFilePath;
    }
}

//------------------------------------------------------------------------------

}  // namespace gabacify
