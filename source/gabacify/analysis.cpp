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

#include "gabac/diff_coding.h"
#include "gabac/encoding.h"
#include "gabac/equality_coding.h"
#include "gabac/lut_transform.h"
#include "gabac/match_coding.h"
#include "gabac/rle_coding.h"

#include "gabacify/configuration.h"
#include "gabacify/encode.h"
#include "gabacify/exceptions.h"
#include "gabacify/helpers.h"
#include "gabacify/log.h"
#include "output_file.h"
#include "input_file.h"


namespace gabacify {

unsigned currentWordsize___;

const std::vector<gabac::SequenceTransformationId> candidateSequenceTransformationIds = {
        gabac::SequenceTransformationId::no_transform,
        gabac::SequenceTransformationId::equality_coding,
        gabac::SequenceTransformationId::match_coding,
        gabac::SequenceTransformationId::rle_coding
};

const std::vector<uint32_t> candidateMatchCodingParameters = {
        32
};

const std::vector<uint32_t> candidateRLECodingParameters = {
        255
};

const std::vector<bool> candidateLUTCodingParameters = {
        false, true
};

const std::vector<bool> candidateDiffParameters{
        false
};


const std::vector<gabac::BinarizationId> candidateUnsignedBinarizationIds = {
        gabac::BinarizationId::BI,
        gabac::BinarizationId::TU,
        gabac::BinarizationId::EG,
        gabac::BinarizationId::TEG,
};

const std::vector<gabac::BinarizationId> candidateSignedBinarizationIds = {
        gabac::BinarizationId::SEG,
        gabac::BinarizationId::STEG
};

const std::vector<unsigned> candidateBinarizationParameters{
        1,
        2,
        7,
        15,
        30
};

//------------------------------------------------------------------------------

const std::vector<gabac::ContextSelectionId> candidateContextSelectionIds = {
        // gabac::ContextSelectionId::bypass,
        // gabac::ContextSelectionId::adaptive_coding_order_0,
        // gabac::ContextSelectionId::adaptive_coding_order_1,
        gabac::ContextSelectionId::adaptive_coding_order_2
};

//------------------------------------------------------------------------------

void getOptimumOfBinarizationParameter(const std::vector<int64_t>& diffTransformedSequence,
                                       gabac::BinarizationId binID,
                                       unsigned binParameter,
                                       std::vector<uint8_t> *const bestByteStream,
                                       TransformedSequenceConfiguration *const bestConfig
){
    size_t bestSize = std::numeric_limits<size_t>::max();

    for (const auto& transID : candidateContextSelectionIds)
    {
        GABACIFY_LOG_DEBUG << "Trying Context: " << unsigned (transID);
        std::vector<uint8_t> currentStream;
        TransformedSequenceConfiguration currentConfiguration;

        gabac::encode(diffTransformedSequence, binID, {binParameter}, transID, &currentStream);
        currentConfiguration.contextSelectionId = transID;

        GABACIFY_LOG_TRACE << "Compressed size with parameter: " << currentStream.size();

        if (currentStream.size() < bestSize)
        {
            GABACIFY_LOG_TRACE << "Found new best context config: " << currentConfiguration.toPrintableString();
            bestSize = currentStream.size();
            *bestByteStream = std::move(currentStream);
            *bestConfig = currentConfiguration;
        }

    }
}

//------------------------------------------------------------------------------

void getOptimumOfBinarization(const std::vector<int64_t>& diffTransformedSequence,
                              gabac::BinarizationId binID,
                              int64_t min, int64_t max,
                              std::vector<uint8_t> *const bestByteStream,
                              TransformedSequenceConfiguration *const bestConfig
){
    size_t bestSize = std::numeric_limits<size_t>::max();

    const std::vector<std::vector<unsigned>> candidates = {{unsigned(std::ceil(std::log2(max)))}, {unsigned(max)}, {0}, {0}, candidateBinarizationParameters, candidateBinarizationParameters};

    for (const auto& transID : candidates[unsigned(binID)])
    {
        GABACIFY_LOG_DEBUG << "Trying Parameter: " << transID;

        if(!binarizationInformation[unsigned(binID)].sbCheck(min, max, transID)) {
            GABACIFY_LOG_DEBUG << "NOT valid for this stream!" << transID;
            continue;
        }

        std::vector<uint8_t> currentStream;
        TransformedSequenceConfiguration currentConfiguration;

        getOptimumOfBinarizationParameter(
                diffTransformedSequence,
                binID,
                transID,
                &currentStream,
                &currentConfiguration
        );
        currentConfiguration.binarizationParameters = {transID};

        GABACIFY_LOG_TRACE << "Compressed size with parameter: " << currentStream.size();

        if (currentStream.size() < bestSize)
        {
            GABACIFY_LOG_TRACE << "Found new best parameter config: " << currentConfiguration.toPrintableString();
            bestSize = currentStream.size();
            *bestByteStream = std::move(currentStream);
            *bestConfig = currentConfiguration;
        }

    }
}

//------------------------------------------------------------------------------

void getOptimumOfDiffTransformedStream(const std::vector<int64_t>& diffTransformedSequence,
                                       unsigned wordsize,
                                       std::vector<uint8_t> *const bestByteStream,
                                       TransformedSequenceConfiguration *const bestConfig
){
    size_t bestSize = std::numeric_limits<size_t>::max();
    int64_t min, max;
    GABACIFY_LOG_TRACE << "Stream analysis: ";
    deriveMinMaxSigned(diffTransformedSequence, wordsize, &min, &max);

    GABACIFY_LOG_TRACE << "Min: " << min << "; Max: " << max;

    std::vector<gabac::BinarizationId>
            candidates = (min >= 0) ? candidateUnsignedBinarizationIds : candidateSignedBinarizationIds;

    if(currentWordsize___ != 1) {
        candidates = {gabac::BinarizationId::TEG};
    }

    for (const auto& transID : candidates)
    {
        GABACIFY_LOG_DEBUG << "Trying Binarization: " << unsigned(transID);

        std::vector<uint8_t> currentStream;
        TransformedSequenceConfiguration currentConfiguration;

        getOptimumOfBinarization(diffTransformedSequence, transID, min, max, &currentStream, &currentConfiguration);
        if(currentStream.size() == 0) {
            continue;
        }
        currentConfiguration.binarizationId = transID;

        GABACIFY_LOG_TRACE << "Size with binarization: " << currentStream.size();

        if (currentStream.size() < bestSize)
        {
            GABACIFY_LOG_TRACE << "Found new best binarization: " << currentConfiguration.toPrintableString();
            bestSize = currentStream.size();
            *bestByteStream = std::move(currentStream);
            *bestConfig = currentConfiguration;
        }

    }
}

//------------------------------------------------------------------------------

void getOptimumOfLutTransformedStream(const std::vector<uint64_t>& lutTransformedSequence,
                                      unsigned wordsize,
                                      std::vector<uint8_t> *const bestByteStream,
                                      TransformedSequenceConfiguration *const bestConfig
){
    size_t bestSize = std::numeric_limits<size_t>::max();
    for (const auto& transID : candidateDiffParameters)
    {
        GABACIFY_LOG_DEBUG << "Trying Diff transformation: " << transID;
        std::vector<uint8_t> currentStream;
        std::vector<int64_t> diffStream;
        TransformedSequenceConfiguration currentConfiguration;

        doDiffTransform(transID, lutTransformedSequence, &diffStream);
        GABACIFY_LOG_DEBUG << "Diff stream (uncompressed): " << diffStream.size() << " bytes";
        getOptimumOfDiffTransformedStream(diffStream, wordsize, &currentStream, &currentConfiguration);
        GABACIFY_LOG_TRACE << "Diff compressed size: " << currentStream.size();
        if(currentStream.size() == 0) {
            continue;
        }

        currentConfiguration.diffCodingEnabled = transID;

        if (currentStream.size() < bestSize)
        {
            GABACIFY_LOG_TRACE << "Found new best diff config: " << currentConfiguration.toPrintableString();
            bestSize = currentStream.size();
            *bestByteStream = std::move(currentStream);
            *bestConfig = currentConfiguration;
        }

    }
}

//------------------------------------------------------------------------------

void getOptimumOfTransformedStream(const std::vector<uint64_t>& transformedSequence,
                                   unsigned wordsize,
                                   std::vector<unsigned char> *const bestByteStream,
                                   TransformedSequenceConfiguration *const bestConfig
){
    size_t bestSize = std::numeric_limits<size_t>::max();
    for (const auto& transID : candidateLUTCodingParameters)
    {
        GABACIFY_LOG_DEBUG << "Trying LUT transformation: " << transID;

        std::vector<uint8_t> lutEnc;
        std::vector<std::vector<uint64_t>> lutStreams;
        TransformedSequenceConfiguration currentConfiguration;

        std::vector<uint8_t> currentStream;

        lutStreams.resize(2);
        doLutTransform(transID, transformedSequence, wordsize, &lutEnc, &lutStreams);
        GABACIFY_LOG_DEBUG << "LutTransformedSequence uncompressed size: " << lutStreams[0].size() << " bytes";
        GABACIFY_LOG_DEBUG << "Lut table (uncompressed): " << lutStreams[1].size() << " bytes";

        getOptimumOfLutTransformedStream(lutStreams[0], wordsize, &currentStream, &currentConfiguration);

        GABACIFY_LOG_TRACE << "Lut transformed compressed size: " << (currentStream.size()+lutEnc.size());

        if(currentStream.size() == 0) {
            continue;
        }
        currentConfiguration.lutTransformationEnabled = transID;

        if ((lutEnc.size() + currentStream.size()) < bestSize)
        {
            GABACIFY_LOG_TRACE << "Found new best LUT config: " << currentConfiguration.toPrintableString();
            bestSize = lutEnc.size() + currentStream.size();
            appendToBytestream(currentStream, &lutEnc);
            *bestByteStream = std::move(lutEnc);
            *bestConfig = currentConfiguration;
        }

    }
}

//------------------------------------------------------------------------------

void getOptimumOfSequenceTransform(const std::vector<uint64_t>& symbols,
                                   const gabac::SequenceTransformationId& transID,
                                   const std::vector<uint32_t>& candidateParameters,
                                   std::vector<unsigned char> *const bestByteStream,
                                   Configuration *const bestConfig
){
    size_t bestByteSize = std::numeric_limits<size_t>::max();
    for (auto const& p : candidateParameters)
    {
        GABACIFY_LOG_DEBUG << "Trying sequence transformation parameter: " << unsigned(p);

        // Execute sequence transform
        std::vector<std::vector<uint64_t>> transformedSequences;
        doSequenceTransform(symbols, transID, p, &transformedSequences);
        GABACIFY_LOG_DEBUG << "Got " << transformedSequences.size() << " transformed sequences";
        for(int i = 0; i< transformedSequences.size(); ++i) {
            GABACIFY_LOG_DEBUG << i << ": " << transformedSequences[i].size() << " bytes";
        }

        // Init configuration
        Configuration currentConf;
        currentConf.sequenceTransformationId = transID;
        currentConf.wordSize = 1;
        currentConf.sequenceTransformationParameter = p;
        currentConf.transformedSequenceConfigurations.resize(transformedSequences.size());

        // Analyze transformed sequences
        std::vector<unsigned char> completeStream;
        bool error = false;
        for (unsigned i = 0; i < transformedSequences.size(); ++i)
        {
            currentWordsize___ = transformationInformation[unsigned(transID)].wordsizes[i];
            GABACIFY_LOG_DEBUG << "Analyzing sequence: " << transformationInformation[unsigned(transID)].streamNames[i] << "";
            std::vector<unsigned char> bestTransformedStream;
            TransformedSequenceConfiguration bestConf;
            getOptimumOfTransformedStream(
                    transformedSequences[i],
                    currentConf.wordSize,
                    &bestTransformedStream,
                    &bestConf
            );

            if(bestTransformedStream.size() == 0) {
                error = true;
                break;
            }

            GABACIFY_LOG_TRACE << "Transformed and compressed sequence size: " << bestTransformedStream.size();

            completeStream.insert(completeStream.end(),bestTransformedStream.begin(), bestTransformedStream.end());
            currentConf.transformedSequenceConfigurations[i] = std::move(bestConf);
        }
        if(error) {
            continue;
        }

        GABACIFY_LOG_TRACE << "With parameter cmoplete transformed size: " << completeStream.size();

        // Update optimum
        if (completeStream.size() < bestByteSize)
        {
            GABACIFY_LOG_DEBUG << "Found new best sequence transform: "
                               << unsigned(transID)
                               << " with size "
                               << completeStream.size();
            bestByteSize = completeStream.size();
            *bestByteStream = std::move(completeStream);
            *bestConfig = currentConf;
        }
    }
}

//------------------------------------------------------------------------------

void getOptimumOfSymbolSequence(const std::vector<uint64_t>& symbols,
                                std::vector<uint8_t> *const bestByteStream,
                                Configuration *const bestConfig
){
    size_t bestSize = std::numeric_limits<size_t>::max();
    for (const auto& transID : candidateSequenceTransformationIds)
    {
        GABACIFY_LOG_DEBUG << "Trying sequence transformation: " << transformationInformation[unsigned(transID)].name;
        const std::vector<uint32_t> candidateDefaultParameters = {0};
        const std::vector<uint32_t> *params[] = {&candidateDefaultParameters,
                                                 &candidateDefaultParameters,
                                                 &candidateMatchCodingParameters,
                                                 &candidateRLECodingParameters};

        std::vector<uint8_t> currentStream;
        Configuration currentConfiguration;
        // Core of analysis
        getOptimumOfSequenceTransform(
                symbols,
                transID,
                *(params[unsigned(transID)]),
                &currentStream,
                &currentConfiguration
        );

        if(currentStream.size() == 0) {
            continue;
        }

        GABACIFY_LOG_TRACE << "Sequence transformed compressed size: " << currentStream.size();

        if (currentStream.size() < bestSize)
        {
            GABACIFY_LOG_TRACE << "Found new best sequence transformation config: " << currentConfiguration.toPrintableString();
            bestSize = currentStream.size();
            *bestByteStream = std::move(currentStream);
            *bestConfig = currentConfiguration;
        }

    }
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

    Configuration bestConfig;
    std::vector<unsigned char> bestByteStream;

    getOptimumOfSymbolSequence(symbols, &bestByteStream, &bestConfig);

    if(bestByteStream.empty()) {
        GABACIFY_DIE("NO CONFIG FOUND");
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
    GABACIFY_LOG_INFO << "Wrote best configuration to: " << configurationFilePath;
}

//------------------------------------------------------------------------------

}  // namespace gabacify

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
