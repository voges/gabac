#include "gabacify/encode.h"

#include <algorithm>
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

//------------------------------------------------------------------------------

// Appends the size of a stream and the actual bytes to bytestream
void appendToBytestream(
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

const std::vector<BinarizationProperties> binarizationInformation = {
        {
                "BI",
                false,
                [](int64_t min, int64_t max, uint64_t parameter) -> bool
                {
                    return min >= 0 && max < (1ll << parameter);
                },
                [](uint64_t min, uint64_t max, uint64_t parameter) -> bool
                {
                    return max < (1ll << parameter);
                }
        },
        {
                "TU",
                false,
                [](int64_t min, int64_t max, uint64_t parameter) -> bool
                {
                    return min >= 0 && max <= parameter;
                },
                [](uint64_t min, uint64_t max, uint64_t parameter) -> bool
                {
                    return max <= parameter;
                }
        },
        {
                "EG",
                false,
                [](int64_t min, int64_t max, uint64_t parameter) -> bool
                {
                    return min >= 0 && max <= ((1ll << 32)-1);
                },
                [](uint64_t min, uint64_t max, uint64_t parameter) -> bool
                {
                    return max <= ((1ll << 32)-1);
                }
        },
        {
                "SEG",
                true,
                [](int64_t min, int64_t max, uint64_t parameter) -> bool
                {
                    return min >= -(1ll << 16) && max <= ((1ll << 16)-1);
                },
                [](uint64_t min, uint64_t max, uint64_t parameter) -> bool
                {
                    return max <= ((1ll << 16)-1);
                }
        },
        {
                "TEG",
                false,
                [](int64_t min, int64_t max, uint64_t parameter) -> bool
                {
                    return min >= 0 && max <= ((1ll << (32-(parameter >> 1)))-1);
                },
                [](uint64_t min, uint64_t max, uint64_t parameter) -> bool
                {
                    return max <= ((1ll << (32-(parameter >> 1)))-1);
                }
        },
        {
                "STEG",
                true,
                [](int64_t min, int64_t max, uint64_t parameter) -> bool
                {
                    return min >= -((1ll << (16-(parameter >> 1)))-1) && max <= ((1ll << (16-(parameter >> 1)))-1);
                },
                [](uint64_t min, uint64_t max, uint64_t parameter) -> bool
                {
                    return max <= ((1ll << (16-(parameter >> 1)))-1);
                }
        }
};

//------------------------------------------------------------------------------

const std::vector<TransformationProperties> transformationInformation = {
        {
                "no_transform", // Name
                {"out"}, // StreamNames
                {0}, // WordSizes (0: non fixed current stream wordsize)
                [](const std::vector<uint64_t>& sequence, uint64_t,
                   std::vector<std::vector<uint64_t>> *const transformedSequences
                )
                {
                    (*transformedSequences)[0] = sequence;
                }
        },
        {
                "equality_coding", // Name
                {"eq_flags",   "raw_symbols"}, // StreamNames
                {1, 0}, // WordSizes (0: non fixed current stream wordsize)
                [](const std::vector<uint64_t>& sequence, uint64_t,
                   std::vector<std::vector<uint64_t>> *const transformedSequences
                )
                {
                    gabac::transformEqualityCoding(
                            sequence,
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1]
                    );
                }
        },
        {
                "match_coding", // Name
                {"pointers",   "lengths", "raw_values"}, // StreamNames
                {4, 4, 0}, // WordSizes (0: non fixed current stream wordsize)
                [](const std::vector<uint64_t>& sequence, uint64_t param,
                   std::vector<std::vector<uint64_t>> *const transformedSequences
                )
                {
                    gabac::transformMatchCoding(
                            sequence,
                            param,
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1],
                            &(*transformedSequences)[2]
                    );
                }
        },
        {
                "rle_coding", // Name
                {"raw_values", "lengths"}, // StreamNames
                {0, 4}, // WordSizes (0: non fixed current stream wordsize)
                [](const std::vector<uint64_t>& sequence, uint64_t param,
                   std::vector<std::vector<uint64_t>> *const transformedSequences
                )
                {
                    gabac::transformRleCoding(
                            sequence,
                            param,
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1]
                    );
                }
        },
        {
                "lut_coding", // Name
                {"sequence",   "lut"}, // StreamNames
                {0, 0}, // WordSizes (0: non fixed current stream wordsize)
                [](const std::vector<uint64_t>& sequence, uint64_t,
                   std::vector<std::vector<uint64_t>> *const transformedSequences
                )
                {
                    gabac::transformLutTransform0(
                            sequence,
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1]
                    );
                }
        },
        {
                "diff_coding", // Name
                {"sequence"}, // StreamNames
                {0}, // WordSizes (0: non fixed current stream wordsize)
                [](const std::vector<uint64_t>&, uint64_t,
                   std::vector<std::vector<uint64_t>> *const
                )
                {
                }
        }
};

//------------------------------------------------------------------------------

static std::vector<unsigned> fixWordSizes(const std::vector<unsigned>& list, unsigned wordsize){
    std::vector<unsigned> ret = list;
    std::replace(ret.begin(), ret.end(), 0u, wordsize);
    return ret;
}

//------------------------------------------------------------------------------

void doSequenceTransform(const std::vector<uint64_t>& sequence,
                         const gabac::SequenceTransformationId& transID,
                         uint64_t param,
                         std::vector<std::vector<uint64_t>> *const transformedSequences
){
    GABACIFY_LOG_TRACE << "Encoding sequence of length: " << sequence.size();

    auto id = unsigned(transID);
    GABACIFY_LOG_DEBUG << "Performing sequence transformation " << transformationInformation[id].name;

    transformedSequences->resize(transformationInformation[id].streamNames.size());
    transformationInformation[id].transform(sequence, param, transformedSequences);

    GABACIFY_LOG_TRACE << "Got " << transformedSequences->size() << " sequences";
    for (int i = 0; i < transformedSequences->size(); ++i) {
        GABACIFY_LOG_TRACE << i << ": " << (*transformedSequences)[i].size() << " bytes";
    }
}

//------------------------------------------------------------------------------

void doLutTransform(bool enabled,
                    const std::vector<uint64_t>& transformedSequence,
                    unsigned int wordSize,
                    std::vector<unsigned char> *const bytestream,
                    std::vector<std::vector<uint64_t >> *const lutSequences
){
    if (!enabled)
    {
        GABACIFY_LOG_TRACE << "LUT transform *dis*abled";
        (*lutSequences)[0] = transformedSequence;
        appendToBytestream({}, bytestream);
        GABACIFY_LOG_DEBUG << "Got uncompressed stream after LUT:" << lutSequences[0].size() << " bytes";
        GABACIFY_LOG_DEBUG << "Got table after LUT:" << lutSequences[1].size() << " bytes";
        return;
    }

    GABACIFY_LOG_TRACE << "LUT transform *en*abled";
    const unsigned LUT_INDEX = 4;
    lutSequences->resize(transformationInformation[LUT_INDEX].streamNames.size());
    transformationInformation[LUT_INDEX].transform(transformedSequence, 0, lutSequences);

    GABACIFY_LOG_DEBUG << "Got uncompressed stream after LUT:" << (*lutSequences)[0].size() << " bytes";
    GABACIFY_LOG_DEBUG << "Got table after LUT:" << (*lutSequences)[1].size() << " bytes";

    // GABACIFY_LOG_DEBUG<<"lut size before coding: "<<inverseLutTmp
    auto *data = (int64_t *) (lutSequences->at(1).data());
    std::vector<unsigned char> inverseLutBitstream;
    gabac::encode(
            std::vector<int64_t>(data, data + (*lutSequences)[1].size()),
            gabac::BinarizationId::BI,
            {wordSize * 8},
            gabac::ContextSelectionId::bypass,
            &inverseLutBitstream
    );


    appendToBytestream(inverseLutBitstream, bytestream);
    GABACIFY_LOG_TRACE << "Wrote LUT bitstream with size: " << inverseLutBitstream.size();
}

//------------------------------------------------------------------------------

void doDiffTransform(bool enabled,
                     const std::vector<uint64_t>& lutTransformedSequence,
                     std::vector<int64_t> *const diffAndLutTransformedSequence
){
    // Diff coding
    if (enabled)
    {
        GABACIFY_LOG_TRACE << "Diff coding *en*abled";
        gabac::transformDiffCoding(lutTransformedSequence, diffAndLutTransformedSequence);
        GABACIFY_LOG_DEBUG << "Got uncompressed stream after diff:" << diffAndLutTransformedSequence->size() << " bytes";
        return;
    }

    diffAndLutTransformedSequence->reserve(lutTransformedSequence.size());

    GABACIFY_LOG_TRACE << "Diff coding *dis*abled";
    for (const auto& lutTransformedSymbol : lutTransformedSequence)
    {
        assert(lutTransformedSymbol <= std::numeric_limits<int64_t>::max());
        diffAndLutTransformedSequence->push_back(static_cast<int64_t>(lutTransformedSymbol));
    }
    GABACIFY_LOG_DEBUG << "Got uncompressed stream after diff:" << diffAndLutTransformedSequence->size() << " bytes";
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

static void encodeSingleSequence(const unsigned wordsize,
                                 const TransformedSequenceConfiguration& configuration,
                                 std::vector<uint64_t> *const seq,
                                 std::vector<unsigned char> *const bytestream
){
    std::vector<std::vector<uint64_t>> lutTransformedSequences;
    lutTransformedSequences.resize(2);
    doLutTransform(
            configuration.lutTransformationEnabled,
            *seq,
            wordsize,
            bytestream,
            &lutTransformedSequences
    );
    seq->clear();
    seq->shrink_to_fit();

    std::vector<int64_t> diffAndLutTransformedSequence;
    doDiffTransform(
            configuration.diffCodingEnabled,
            lutTransformedSequences[0],
            &diffAndLutTransformedSequence
    );
    lutTransformedSequences[0].clear();
    lutTransformedSequences[0].shrink_to_fit();

    encodeStream(configuration, diffAndLutTransformedSequence, bytestream);
    diffAndLutTransformedSequence.clear();
    diffAndLutTransformedSequence.shrink_to_fit();
}

//------------------------------------------------------------------------------

static void encodeWithConfiguration(
        const Configuration& configuration,
        std::vector<uint64_t> *const sequence,
        std::vector<unsigned char> *const bytestream
){


    std::vector<std::vector<uint64_t>> transformedSequences;
    doSequenceTransform(
            *sequence,
            configuration.sequenceTransformationId,
            configuration.sequenceTransformationParameter,
            &transformedSequences
    );
    sequence->clear();
    sequence->shrink_to_fit();
    std::vector<unsigned> wordsizes = fixWordSizes(
            transformationInformation[unsigned(configuration.sequenceTransformationId)].wordsizes,
            configuration.wordSize
    );

    // Loop through the transformed sequences
    for (size_t i = 0; i < transformedSequences.size(); i++)
    {
        encodeSingleSequence(
                wordsizes[i],
                configuration.transformedSequenceConfigurations.at(i),
                &(transformedSequences[i]),
                bytestream
        );
        transformedSequences[i].clear();
        transformedSequences[i].shrink_to_fit();
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

    encodeWithConfiguration(configuration, &symbols, &buffer);
    symbols.clear();
    symbols.shrink_to_fit();

    // Write the bytestream
    OutputFile outputFile(outputFilePath);
    outputFile.write(&buffer[0], 1, buffer.size());
    GABACIFY_LOG_INFO << "Wrote bytestream of size " << buffer.size() << " to: " << outputFilePath;
    buffer.clear();
    buffer.shrink_to_fit();
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

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------