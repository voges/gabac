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
            //           gabac::SequenceTransformationId::equality_coding,
            //              gabac::SequenceTransformationId::match_coding,
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

void getOptimumOfBinarizationParameter(const gabac::DataStream& diffTransformedSequence,
                                       gabac::BinarizationId binID,
                                       unsigned binParameter,
                                       gabac::DataStream *const bestByteStream,
                                       const gabac::DataStream& lut,
                                       TransformedSequenceConfiguration *const bestConfig,
                                       TransformedSequenceConfiguration *const currentConfig
){

    for (const auto& transID : getCandidateConfig().candidateContextSelectionIds)
    {
        GABACIFY_LOG_TRACE << "Trying Context: " << unsigned(transID);
        gabac::DataStream currentStream(0, 1);

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

void getOptimumOfBinarization(const gabac::DataStream& diffTransformedSequence,
                              gabac::BinarizationId binID,
                              int64_t min, int64_t max,
                              gabac::DataStream *const bestByteStream,
                              const gabac::DataStream& lut,
                              TransformedSequenceConfiguration *const bestConfig,
                              TransformedSequenceConfiguration *const currentConfig
){

    const unsigned BIPARAM = (max > 0) ? unsigned(std::floor(std::log2(max))+1) : 1;
    const unsigned TUPARAM = (max > 0) ? max : 1;
    const std::vector<std::vector<unsigned>> candidates = {{std::min(BIPARAM, 32u)},
                                                           {std::min(TUPARAM, 32u)},
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

void getOptimumOfDiffTransformedStream(const gabac::DataStream& diffTransformedSequence,
                                       unsigned wordsize,
                                       gabac::DataStream *const bestByteStream,
                                       const gabac::DataStream& lut,
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

void getOptimumOfLutTransformedStream(gabac::DataStream& lutTransformedSequence,
                                      unsigned wordsize,
                                      gabac::DataStream *const bestByteStream,
                                      const gabac::DataStream& lut,
                                      TransformedSequenceConfiguration *const bestConfig,
                                      TransformedSequenceConfiguration *const currentConfig
){
    for (const auto& transID : getCandidateConfig().candidateDiffParameters)
    {
        GABACIFY_LOG_DEBUG << "Trying Diff transformation: " << transID;
        gabac::DataStream diffStream(0, wordsize);

//REPLACE        doDiffTransform(transID, lutTransformedSequence, &diffStream);
        GABACIFY_LOG_DEBUG << "Diff stream (uncompressed): " << diffStream.size() << " bytes";
        currentConfig->diffCodingEnabled = transID;
        getOptimumOfDiffTransformedStream(diffStream, wordsize, bestByteStream, lut, bestConfig, currentConfig);
    }
}

//------------------------------------------------------------------------------

void getOptimumOfLutOrder(const gabac::DataStream& transformedSequence,
                          unsigned wordsize,
                          gabac::DataStream *const bestByteStream,
                          TransformedSequenceConfiguration *const bestConfig,
                          TransformedSequenceConfiguration *const currentConfig
){
    for (const auto& transID : getCandidateConfig().candidateLUTCodingParameters)
    {

        if(transID == false && currentConfig->lutOrder != 0) {
            continue;
        }

        GABACIFY_LOG_DEBUG << "Trying LUT transformation: " << transID;

        gabac::DataStream lutEnc(0, wordsize);
        std::vector<gabac::DataStream> lutStreams;

        lutStreams.resize(3);

        currentConfig->lutBits = 0;
        currentConfig->lutTransformationEnabled = transID;

        gabac::DataStream g = transformedSequence;

 /*REPLACE       doLutTransform(
                transID,
                g,
                currentConfig->lutOrder,
                &lutEnc,
                &lutStreams,
                &currentConfig->lutBits
        );*/
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

void getOptimumOfTransformedStream(const gabac::DataStream& transformedSequence,
                                   unsigned wordsize,
                                   gabac::DataStream *const bestByteStream,
                                   TransformedSequenceConfiguration *const bestConfig
){
    for (const auto& transID : getCandidateConfig().candidateLutOrder)
    {
        GABACIFY_LOG_TRACE << "Trying LUT order: " << transID;
        TransformedSequenceConfiguration currentConfiguration;
        currentConfiguration.lutOrder = transID;
        getOptimumOfLutOrder(transformedSequence, wordsize, bestByteStream, bestConfig, &currentConfiguration);
    }
}

//------------------------------------------------------------------------------

void getOptimumOfSequenceTransform(const gabac::DataStream& symbols,
                                   const std::vector<uint32_t>& candidateParameters,
                                   gabac::DataStream *const bestByteStream,
                                   Configuration *const bestConfig,
                                   Configuration *const currentConfig
){
    for (auto const& p : candidateParameters)
    {
        GABACIFY_LOG_DEBUG << "Trying sequence transformation parameter: " << unsigned(p);

        // Execute sequence transform
        std::vector<gabac::DataStream> transformedSequences;
        gabac::DataStream s = symbols;
//REPLACE        doSequenceTransform(s, currentConfig->sequenceTransformationId, p, &transformedSequences);
        GABACIFY_LOG_DEBUG << "Got " << transformedSequences.size() << " transformed sequences";
        for (unsigned i = 0; i < transformedSequences.size(); ++i)
        {
            GABACIFY_LOG_DEBUG << i << ": " << transformedSequences[i].size() << " bytes";
        }


        currentConfig->sequenceTransformationParameter = p;
        currentConfig->transformedSequenceConfigurations.resize(transformedSequences.size());

        // Analyze transformed sequences
        gabac::DataStream completeStream(0, 1);
        bool error = false;
        for (unsigned i = 0; i < transformedSequences.size(); ++i)
        {
            unsigned currWordSize = 0;/*gabac::fixWordSizes(
                    gabac::transformationInformation[unsigned(currentConfig->sequenceTransformationId)].wordsizes,
                    currentConfig->wordSize
            )[i];*/
            GABACIFY_LOG_DEBUG << "Analyzing sequence: "
                               << gabac::transformationInformation[unsigned(currentConfig->sequenceTransformationId)].streamNames[i]
                               << "";
            gabac::DataStream bestTransformedStream(0, 1);
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

void getOptimumOfSymbolSequence(const gabac::DataStream& symbols,
                                gabac::DataStream *const bestByteStream,
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
    gabac::DataStream bestByteStream(0, 1);
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

        gabac::DataStream buffer(inputFile.size(), 1);
        inputFile.read(buffer.getData(), 1, buffer.size());

        Configuration currentConfig;

        currentConfig.wordSize = w;

        // Generate symbol stream from byte buffer
        gabac::DataStream symbols(0, w);
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
    outputFile.write(bestByteStream.getData(), 1, bestByteStream.size());
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
