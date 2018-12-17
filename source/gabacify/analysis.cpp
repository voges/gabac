#include "gabacify/analysis.h"

#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <cstdio>

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <limits>
#include <utility>
#include <string>
#include <vector>

#include "gabac/constants.h"
#include "gabac/encoding.h"

#include "gabacify/configuration.h"
#include "gabacify/encode.h"
#include "gabacify/exceptions.h"
#include "gabacify/helpers.h"
#include "gabacify/log.h"
#include "output_file.h"
#include "input_file.h"


namespace gabacify {

struct CandidateConfig
{
    std::vector<unsigned> candidateWordsizes;
    std::vector<gabac::SequenceTransformationId> candidateSequenceTransformationIds;
    std::vector<uint32_t> candidateMatchCodingParameters;
    std::vector<uint32_t> candidateRLECodingParameters;
    std::vector<bool> candidateLUTCodingParameters;
    std::vector<bool> candidateDiffParameters;
    std::vector<gabac::BinarizationId> candidateUnsignedBinarizationIds;
    std::vector<gabac::BinarizationId> candidateSignedBinarizationIds;
    std::vector<unsigned> candidateBinarizationParameters;
    std::vector<gabac::ContextSelectionId> candidateContextSelectionIds;
    std::vector<unsigned> candidateLutOrder;
};

static const CandidateConfig& getCandidateConfig(){
    static const CandidateConfig config = {
            { // Wordsizes
                    1,
                    4
            },
            { // Sequence Transformations
                    gabac::SequenceTransformationId::no_transform,
                       gabac::SequenceTransformationId::equality_coding,
                          gabac::SequenceTransformationId::match_coding,
                             gabac::SequenceTransformationId::rle_coding
            },
            { // Match coding window sizes
                    32,
                       256
            },
            { // RLE Guard
                    255
            },
            { // LUT transform
                    false,
                       true
            },
            { // Diff transform
                    false//,
                    //true
            },
            { // Binarizations (unsigned)
                    gabac::BinarizationId::BI,
                       gabac::BinarizationId::TU,
                          gabac::BinarizationId::EG,
                             gabac::BinarizationId::TEG
            },
            { // Binarizations (signed)
                    gabac::BinarizationId::SEG,
                       gabac::BinarizationId::STEG
            },
            { // Binarization parameters (TEG and STEG only)
                    1, 2, 3, 5, 7, 9,
                    15, 30, 255
            },
            { // Context modes
                    // gabac::ContextSelectionId::bypass,
                    gabac::ContextSelectionId::adaptive_coding_order_0,
                       gabac::ContextSelectionId::adaptive_coding_order_1,
                          gabac::ContextSelectionId::adaptive_coding_order_2
            },
            { // LUT order
                    0,
                       1
            }
    };
    return config;
}

//------------------------------------------------------------------------------

void getOptimumOfBinarizationParameter(const std::vector<int64_t>& diffTransformedSequence,
                                       gabac::BinarizationId binID,
                                       unsigned binParameter,
                                       std::vector<uint8_t> *const bestByteStream,
                                       const std::vector<uint8_t>& lut,
                                       TransformedSequenceConfiguration *const bestConfig,
                                       TransformedSequenceConfiguration *const currentConfig
){

    for (const auto& transID : getCandidateConfig().candidateContextSelectionIds)
    {
        GABACIFY_LOG_TRACE << "Trying Context: " << unsigned(transID);
        std::vector<uint8_t> currentStream;

        currentConfig->contextSelectionId = transID;
        gabac::encode(diffTransformedSequence, binID, {binParameter}, transID, &currentStream);

        GABACIFY_LOG_TRACE << "Compressed size with parameter: " << currentStream.size();

        if ((currentStream.size() + lut.size() + 4 < bestByteStream->size()) || bestByteStream->empty())
        {
            GABACIFY_LOG_TRACE << "Found new best context config: " << currentConfig->toPrintableString();
            *bestByteStream = lut;
            appendToBytestream(currentStream, bestByteStream);
            *bestConfig = *currentConfig;
        }

    }
}

//------------------------------------------------------------------------------

void getOptimumOfBinarization(const std::vector<int64_t>& diffTransformedSequence,
                              gabac::BinarizationId binID,
                              int64_t min, int64_t max,
                              std::vector<uint8_t> *const bestByteStream,
                              const std::vector<uint8_t>& lut,
                              TransformedSequenceConfiguration *const bestConfig,
                              TransformedSequenceConfiguration *const currentConfig
){

    const unsigned BIPARAM = (max > 0) ? unsigned(std::ceil(std::log2(max))) : 1;
    const std::vector<std::vector<unsigned>> candidates = {{std::min(BIPARAM, 63u)},
                                                           {std::min(unsigned(max), 255u)},
                                                           {0},
                                                           {0},
                                                           getCandidateConfig().candidateBinarizationParameters,
                                                           getCandidateConfig().candidateBinarizationParameters};

    for (const auto& transID : candidates[unsigned(binID)])
    {
        GABACIFY_LOG_TRACE << "Trying Parameter: " << transID;

        if (!gabac::binarizationInformation[unsigned(binID)].sbCheck(min, max, transID))
        {
            GABACIFY_LOG_TRACE << "NOT valid for this stream!" << transID;
            continue;
        }

        currentConfig->binarizationParameters = {transID};

        getOptimumOfBinarizationParameter(
                diffTransformedSequence,
                binID,
                transID,
                bestByteStream,
                lut,
                bestConfig,
                currentConfig
        );

    }
}

//------------------------------------------------------------------------------

void getOptimumOfDiffTransformedStream(const std::vector<int64_t>& diffTransformedSequence,
                                       unsigned wordsize,
                                       std::vector<uint8_t> *const bestByteStream,
                                       const std::vector<uint8_t>& lut,
                                       TransformedSequenceConfiguration *const bestConfig,
                                       TransformedSequenceConfiguration *const currentConfig
){
    int64_t min, max;
    GABACIFY_LOG_TRACE << "Stream analysis: ";
    deriveMinMaxSigned(diffTransformedSequence, wordsize, &min, &max);

    GABACIFY_LOG_TRACE << "Min: " << min << "; Max: " << max;

    std::vector<gabac::BinarizationId>
            candidates = (min >= 0)
                         ? getCandidateConfig().candidateUnsignedBinarizationIds
                         : getCandidateConfig().candidateSignedBinarizationIds; // TODO: avoid copy

    for (const auto& transID : candidates)
    {
        GABACIFY_LOG_TRACE << "Trying Binarization: " << unsigned(transID);


        currentConfig->binarizationId = transID;
        getOptimumOfBinarization(
                diffTransformedSequence,
                transID,
                min,
                max,
                bestByteStream,
                lut,
                bestConfig,
                currentConfig
        );

    }
}

//------------------------------------------------------------------------------

void getOptimumOfLutTransformedStream(const std::vector<uint64_t>& lutTransformedSequence,
                                      unsigned wordsize,
                                      std::vector<uint8_t> *const bestByteStream,
                                      const std::vector<uint8_t>& lut,
                                      TransformedSequenceConfiguration *const bestConfig,
                                      TransformedSequenceConfiguration *const currentConfig
){
    for (const auto& transID : getCandidateConfig().candidateDiffParameters)
    {
        GABACIFY_LOG_DEBUG << "Trying Diff transformation: " << transID;
        std::vector<int64_t> diffStream;

        doDiffTransform(transID, lutTransformedSequence, &diffStream);
        GABACIFY_LOG_DEBUG << "Diff stream (uncompressed): " << diffStream.size() << " bytes";
        currentConfig->diffCodingEnabled = transID;
        getOptimumOfDiffTransformedStream(diffStream, wordsize, bestByteStream, lut, bestConfig, currentConfig);
    }
}

//------------------------------------------------------------------------------

void getOptimumOfLutOrder(const std::vector<uint64_t>& transformedSequence,
                          unsigned wordsize,
                          std::vector<unsigned char> *const bestByteStream,
                          TransformedSequenceConfiguration *const bestConfig,
                          TransformedSequenceConfiguration *const currentConfig
){
    for (const auto& transID : getCandidateConfig().candidateLUTCodingParameters)
    {

        if(transID == false && currentConfig->lutOrder != 0) {
            continue;
        }

        GABACIFY_LOG_DEBUG << "Trying LUT transformation: " << transID;

        std::vector<uint8_t> lutEnc;
        std::vector<std::vector<uint64_t>> lutStreams;

        lutStreams.resize(3);

        currentConfig->lutBits = 0;
        currentConfig->lutTransformationEnabled = transID;

        doLutTransform(
                transID,
                transformedSequence,
                currentConfig->lutOrder,
                &lutEnc,
                &lutStreams,
                &currentConfig->lutBits
        );
        if (lutStreams[0].size() != transformedSequence.size())
        {
            GABACIFY_LOG_DEBUG << "Lut transformed failed. Probably the symbol space is too large. Skipping. ";
            continue;
        }
        GABACIFY_LOG_DEBUG << "LutTransformedSequence uncompressed size: " << lutStreams[0].size() << " bytes";
        GABACIFY_LOG_DEBUG << "Lut table (uncompressed): " << lutStreams[1].size() << " bytes";
        GABACIFY_LOG_DEBUG << "Lut table1 (uncompressed): " << lutStreams[2].size() << " bytes";

        getOptimumOfLutTransformedStream(
                lutStreams[0],
                wordsize,
                bestByteStream,
                lutEnc,
                bestConfig,
                currentConfig
        );

    }
}

//------------------------------------------------------------------------------

void getOptimumOfTransformedStream(const std::vector<uint64_t>& transformedSequence,
                                   unsigned wordsize,
                                   std::vector<unsigned char> *const bestByteStream,
                                   TransformedSequenceConfiguration *const bestConfig
){
    for (const auto& transID : getCandidateConfig().candidateLutOrder)
    {
        TransformedSequenceConfiguration currentConfiguration;
        currentConfiguration.lutOrder = transID;
        getOptimumOfLutOrder(transformedSequence, wordsize, bestByteStream, bestConfig, &currentConfiguration);
    }
}

//------------------------------------------------------------------------------

void getOptimumOfSequenceTransform(const std::vector<uint64_t>& symbols,
                                   const std::vector<uint32_t>& candidateParameters,
                                   std::vector<unsigned char> *const bestByteStream,
                                   Configuration *const bestConfig,
                                   Configuration *const currentConfig
){
    for (auto const& p : candidateParameters)
    {
        GABACIFY_LOG_DEBUG << "Trying sequence transformation parameter: " << unsigned(p);

        // Execute sequence transform
        std::vector<std::vector<uint64_t>> transformedSequences;
        doSequenceTransform(symbols, currentConfig->sequenceTransformationId, p, &transformedSequences);
        GABACIFY_LOG_DEBUG << "Got " << transformedSequences.size() << " transformed sequences";
        for (unsigned i = 0; i < transformedSequences.size(); ++i)
        {
            GABACIFY_LOG_DEBUG << i << ": " << transformedSequences[i].size() << " bytes";
        }


        currentConfig->sequenceTransformationParameter = p;
        currentConfig->transformedSequenceConfigurations.resize(transformedSequences.size());

        // Analyze transformed sequences
        std::vector<unsigned char> completeStream;
        bool error = false;
        for (unsigned i = 0; i < transformedSequences.size(); ++i)
        {
            unsigned currWordSize = gabac::fixWordSizes(
                    gabac::transformationInformation[unsigned(currentConfig->sequenceTransformationId)].wordsizes,
                    currentConfig->wordSize
            )[i];
            GABACIFY_LOG_DEBUG << "Analyzing sequence: "
                               << gabac::transformationInformation[unsigned(currentConfig->sequenceTransformationId)].streamNames[i]
                               << "";
            std::vector<unsigned char> bestTransformedStream;
            getOptimumOfTransformedStream(
                    transformedSequences[i],
                    currWordSize,
                    &bestTransformedStream,
                    &(*currentConfig).transformedSequenceConfigurations[i]
            );

            if (bestTransformedStream.empty())
            {
                error = true;
                break;
            }

            GABACIFY_LOG_TRACE << "Transformed and compressed sequence size: " << bestTransformedStream.size();

            //appendToBytestream(bestTransformedStream, &completeStream);
            completeStream.insert(completeStream.end(), bestTransformedStream.begin(), bestTransformedStream.end());

            if ((completeStream.size() >= bestByteStream->size()) &&
                (!bestByteStream->empty()))
            {
                GABACIFY_LOG_TRACE << "Already bigger stream than current maximum (Sequence transform level): Skipping "
                                   << bestTransformedStream.size();
                error = true;
                break;
            }
        }
        if (error)
        {
            GABACIFY_LOG_DEBUG << "Could not find working configuration for this stream, or smaller stream exists. skipping: "
                               << completeStream.size();
            continue;
        }

        GABACIFY_LOG_TRACE << "With parameter complete transformed size: " << completeStream.size();

        // Update optimum
        if (completeStream.size() < bestByteStream->size() || bestByteStream->empty())
        {
            GABACIFY_LOG_DEBUG << "Found new best sequence transform: "
                               << unsigned(currentConfig->sequenceTransformationId)
                               << " with size "
                               << completeStream.size();
            *bestByteStream = std::move(completeStream);
            *bestConfig = *currentConfig;
        }
    }
}

//------------------------------------------------------------------------------

void getOptimumOfSymbolSequence(const std::vector<uint64_t>& symbols,
                                std::vector<uint8_t> *const bestByteStream,
                                Configuration *const bestConfig,
                                Configuration *const currentConfiguration
){
    const std::vector<uint32_t> candidateDefaultParameters = {0};
    const std::vector<uint32_t> *params[] = {&candidateDefaultParameters,
                                             &candidateDefaultParameters,
                                             &getCandidateConfig().candidateMatchCodingParameters,
                                             &getCandidateConfig().candidateRLECodingParameters};
    for (const auto& transID : getCandidateConfig().candidateSequenceTransformationIds)
    {
        GABACIFY_LOG_DEBUG << "Trying sequence transformation: "
                           << gabac::transformationInformation[unsigned(transID)].name;

        currentConfiguration->sequenceTransformationId = transID;
        // Core of analysis
        getOptimumOfSequenceTransform(
                symbols,
                *(params[unsigned(transID)]),
                bestByteStream,
                bestConfig,
                currentConfiguration
        );

        GABACIFY_LOG_TRACE << "Sequence transformed compressed size: " << bestByteStream->size();
    }
}

//------------------------------------------------------------------------------

void encode_analyze(const std::string& inputFilePath,
                    const std::string& configurationFilePath,
                    const std::string& outputFilePath
){
    Configuration bestConfig;
    std::vector<unsigned char> bestByteStream;
    for (const auto& w : getCandidateConfig().candidateWordsizes)
    {

        InputFile inputFile(inputFilePath);

        if (inputFile.size() % w != 0)
        {
            GABACIFY_LOG_INFO << "Input stream size "
                              << inputFile.size()
                              << " is not a multiple of word size "
                              << w
                              << "! Skipping word size.";
            continue;
        }

        std::vector<unsigned char> buffer(inputFile.size());
        inputFile.read(&buffer[0], 1, buffer.size());

        Configuration currentConfig;

        currentConfig.wordSize = w;

        // Generate symbol stream from byte buffer
        std::vector<uint64_t> symbols;
        generateSymbolStream(buffer, w, &symbols);
        buffer.clear();
        buffer.shrink_to_fit();


        getOptimumOfSymbolSequence(symbols, &bestByteStream, &bestConfig, &currentConfig);

        if (bestByteStream.empty())
        {
            GABACIFY_DIE("NO CONFIG FOUND");
        }
    }

    // Write the smallest bytestream
    OutputFile outputFile(outputFilePath);
    outputFile.write(&bestByteStream[0], 1, bestByteStream.size());
    GABACIFY_LOG_INFO << "Wrote smallest bytestream of size "
                      << bestByteStream.size()
                      << " to: "
                      << outputFilePath;

    // Write the best configuration as JSON
    std::string jsonString = bestConfig.toJsonString();
    OutputFile configurationFile(configurationFilePath);
    configurationFile.write(&jsonString[0], 1, jsonString.size());
    GABACIFY_LOG_DEBUG << "with configuration: \n"
                       << bestConfig.toPrintableString();
    GABACIFY_LOG_INFO << "Wrote best configuration to: " << configurationFilePath;
}

//------------------------------------------------------------------------------

}  // namespace gabacify

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
