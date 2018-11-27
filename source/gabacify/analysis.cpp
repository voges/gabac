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
#include "gabac/equality_coding.h"
#include "gabac/lut_transform.h"
#include "gabac/match_coding.h"
#include "gabac/rle_coding.h"

#include "gabacify/configuration.h"
#include "gabacify/exceptions.h"
#include "gabacify/helpers.h"
#include "gabacify/log.h"


namespace gabacify {


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

const std::vector<uint32_t> candidateLUTCodingParameters = {
    0, 1
};

const bool enableDiffCoding = false;

const std::vector<gabac::BinarizationId> candidateBinarizationIds = {
    gabac::BinarizationId::BI,
    gabac::BinarizationId::TU,
    gabac::BinarizationId::EG,
    gabac::BinarizationId::SEG,
    gabac::BinarizationId::TEG,
    gabac::BinarizationId::STEG
};

const std::vector<uint8_t> candidateBinarizationParameters {
    1,
    2,
    7,
    15,
    30
};

const std::vector<gabac::ContextSelectionId> candidateContextSelectionIds = {
    // gabac::ContextSelectionId::bypass,
    // gabac::ContextSelectionId::adaptive_coding_order_0,
    // gabac::ContextSelectionId::adaptive_coding_order_1,
    gabac::ContextSelectionId::adaptive_coding_order_2
};


static void addValidContextSelectionIds(
        unsigned int transformedSequenceId,
        std::vector<Configuration> * const configurations,
        Configuration * const configuration
){
    assert(configurations != nullptr);
    assert(configuration != nullptr);

    for (const auto& contextSelectionId : candidateContextSelectionIds)
    {
        configuration->transformedSequenceConfigurations[transformedSequenceId].contextSelectionId = contextSelectionId;

        // Check if 'configuration' is complete
        // bool complete = false;
        // if (transformedSequenceId == 0)
        // {
        //     complete = true;
        // }
        // else if (configuration->sequenceTransformationId == gabac::SequenceTransformationId::equality_coding || configuration->sequenceTransformationId == gabac::SequenceTransformationId::rle_coding)
        // {
        //     if (transformedSequenceId == 1)
        //     {
        //         complete = true;
        //     }
        // }
        // else if (configuration->sequenceTransformationId == gabac::SequenceTransformationId::match_coding)
        // {
        //     if (transformedSequenceId == 2)
        //     {
        //         complete = true;
        //     }
        // }
        // else
        // {
        //     GABACIFY_DIE("Invalid transformedSequenceId");
        // }
        //
        // if (complete)
        // {
            configurations->push_back(*configuration);
        // }
    }
}


static void addValidUnsignedBinarizations(
        const std::vector<uint64_t>& diffTransformedSequence,
        unsigned int transformedSequenceId,
        unsigned int wordSize,
        std::vector<Configuration> * const configurations,
        Configuration * const configuration
){
    uint64_t min_value;
    uint64_t max_value;
    std::vector<unsigned int> binarizationParameters;

    deriveMinMaxUnsigned(diffTransformedSequence, wordSize, &min_value, &max_value);

    // Identify and add allowed binarizations
    for (auto binarizationId : candidateBinarizationIds)
    {
        binarizationParameters.clear();
        configuration->transformedSequenceConfigurations[transformedSequenceId].binarizationId = binarizationId;
        if (binarizationId == gabac::BinarizationId::BI
            && max_value != 1
            // The following three lines mean: do not use BI in the case that
            // - rle_coding AND lengths
            // - match_coding AND (pointers OR values)
            && !(configuration->sequenceTransformationId == gabac::SequenceTransformationId::rle_coding && transformedSequenceId != 0)
            && !(configuration->sequenceTransformationId == gabac::SequenceTransformationId::match_coding && (transformedSequenceId != 0 || transformedSequenceId != 1)))
        {
            binarizationParameters.push_back(std::max(1u, static_cast<unsigned int>(floor(log2(max_value)) + 1)));
            configuration->transformedSequenceConfigurations[transformedSequenceId].binarizationParameters = binarizationParameters;
            addValidContextSelectionIds(
                    transformedSequenceId,
                    configurations,
                    configuration
            );
        }
        else if (binarizationId == gabac::BinarizationId::TU
                 && max_value <= 32
                 && !(configuration->sequenceTransformationId == gabac::SequenceTransformationId::rle_coding && transformedSequenceId != 0)
                 && !(configuration->sequenceTransformationId == gabac::SequenceTransformationId::match_coding && (transformedSequenceId != 0 || transformedSequenceId != 1)))
        {
            binarizationParameters.push_back(
                    std::max(
                            static_cast<unsigned int>(1),
                            static_cast<unsigned int>(max_value)));
            configuration->transformedSequenceConfigurations[transformedSequenceId].binarizationParameters = binarizationParameters;
            addValidContextSelectionIds(
                    transformedSequenceId,
                    configurations,
                    configuration
            );
        }
        else if (binarizationId == gabac::BinarizationId::EG
                 && max_value < 65535
                 && !(configuration->sequenceTransformationId == gabac::SequenceTransformationId::rle_coding && transformedSequenceId != 0)
                 && !(configuration->sequenceTransformationId == gabac::SequenceTransformationId::match_coding && (transformedSequenceId != 0 || transformedSequenceId != 1)))
        {
            // binarizationParameters.push_back(0);
            configuration->transformedSequenceConfigurations[transformedSequenceId].binarizationParameters = binarizationParameters;
            addValidContextSelectionIds(
                    transformedSequenceId,
                    configurations,
                    configuration
            );
        }
        else if (binarizationId == gabac::BinarizationId::TEG)
        {
            for (unsigned char binarizationParameter : candidateBinarizationParameters)
            {
                binarizationParameters.clear();
                if (max_value <= 65535u + binarizationParameter && max_value > binarizationParameter)
                {
                    binarizationParameters.push_back(binarizationParameter);
                    configuration->transformedSequenceConfigurations[transformedSequenceId].binarizationParameters = binarizationParameters;
                    addValidContextSelectionIds(
                            transformedSequenceId,
                            configurations,
                            configuration
                    );
                }
            }
        }
    }
}


static void addValidSignedBinarizations(
        const std::vector<int64_t>& diffTransformedSequence,
        unsigned int transformedSequenceId,
        unsigned int wordSize,
        std::vector<Configuration> *configurations,
        Configuration *configuration
){
    int64_t min_value;
    int64_t max_value;
    std::vector<unsigned int> binarizationParameters;

    deriveMinMaxSigned(diffTransformedSequence, wordSize, &min_value, &max_value);

    // Identify and add allowed binarizations
    for (auto binarizationId : candidateBinarizationIds)
    {
        configuration->transformedSequenceConfigurations[transformedSequenceId].binarizationId = binarizationId;
        if (binarizationId == gabac::BinarizationId::SEG && min_value >= -32767 && max_value <= 32767)
        {
            // binarizationParameters.push_back(0);
            configuration->transformedSequenceConfigurations[transformedSequenceId].binarizationParameters = binarizationParameters;
            addValidContextSelectionIds(
                    transformedSequenceId,
                    configurations,
                    configuration
            );
        }
        else if (binarizationId == gabac::BinarizationId::STEG)
        {
            for (unsigned char binarizationParameter : candidateBinarizationParameters)
            {
                if (min_value >= -32767 - binarizationParameter &&
                    max_value <= 32767 + binarizationParameter &&
                    (abs(min_value) > binarizationParameter || abs(max_value) > binarizationParameter))
                {
                    binarizationParameters.clear();
                    binarizationParameters.push_back(binarizationParameter);
                    configuration->transformedSequenceConfigurations[transformedSequenceId].binarizationParameters = binarizationParameters;
                    addValidContextSelectionIds(
                            transformedSequenceId,
                            configurations,
                            configuration
                    );
                }
            }
        }
    }
}


static void addDiffTransformations(const std::vector<uint64_t>& lutTransformedSequence,
                                   unsigned int transformedSequenceId,
                                   unsigned int wordSize,
                                   std::vector<Configuration> *configurations,
                                   Configuration *configuration
){
    configuration->transformedSequenceConfigurations[transformedSequenceId].diffCodingEnabled = false;
    addValidUnsignedBinarizations(
            lutTransformedSequence,
            transformedSequenceId,
            wordSize,
            configurations,
            configuration
    );

    if (enableDiffCoding)
    {
        std::vector<int64_t> transformedSymbols;
        configuration->transformedSequenceConfigurations[transformedSequenceId].diffCodingEnabled = true;
        gabac::transformDiffCoding(lutTransformedSequence, &transformedSymbols);
        addValidSignedBinarizations(
                transformedSymbols,
                transformedSequenceId,
                wordSize,
                configurations,
                configuration
        );
    }
}


static void addLutTransformations(const std::vector<uint64_t>& sequenceTransformedSequence,
                                  unsigned int transformedSequenceId,
                                  unsigned int wordSize,
                                  std::vector<Configuration> *configurations,
                                  Configuration *configuration
){
    std::vector<uint64_t> transformedSymbols;
    std::vector<uint64_t> inverseLut0;


    for (auto& lutCodingParameter : candidateLUTCodingParameters)
    {
        if (lutCodingParameter == 0)
        {
            configuration->transformedSequenceConfigurations[transformedSequenceId]
                    .lutTransformationEnabled = false;
            configuration->transformedSequenceConfigurations[transformedSequenceId]
                    .lutTransformationParameter = 0;
        }
        else
        {
            configuration->transformedSequenceConfigurations[transformedSequenceId]
                    .lutTransformationEnabled = true;
            configuration->transformedSequenceConfigurations[transformedSequenceId]
                    .lutTransformationParameter = lutCodingParameter - 1;
        }

        switch (lutCodingParameter)
        {
            case 0:  // no lut_transform
                addDiffTransformations(
                        sequenceTransformedSequence,
                        transformedSequenceId,
                        wordSize,
                        configurations,
                        configuration
                );
                break;
            case 1:  // order0 lut_transform
                gabac::transformLutTransform0(sequenceTransformedSequence, &transformedSymbols, &inverseLut0);
                // for values
                addDiffTransformations(
                        transformedSymbols,
                        transformedSequenceId,
                        wordSize,
                        configurations,
                        configuration
                );
                break;
            default:
                // TODO(Jan): return GABAC_FAILURE;
                break;
        }
    }
}


static void addSubsequences(const std::vector<std::vector<uint64_t>>& sequenceTransformedSequences,
                            std::vector<unsigned int> wordSize,
                            std::vector<Configuration> *configurations,
                            Configuration *configuration
){
    std::vector<Configuration> temp_configs;
    std::vector<Configuration> temp_configs2;
    temp_configs.push_back(*configuration);

    assert(sequenceTransformedSequences.size() <= std::numeric_limits<unsigned int>::max());
    for (unsigned int i = 0; i < sequenceTransformedSequences.size(); i++)
    {
        GABACIFY_LOG_INFO << "Defining configurations for subsequence " << i;
        temp_configs2.clear();
        for (auto& temp_config : temp_configs)
        {
            addLutTransformations(
                    sequenceTransformedSequences.at(i),
                    i,
                    wordSize.at(i),
                    &temp_configs2,
                    &temp_config
            );
        }
        temp_configs = temp_configs2;
    }
    configurations->insert(configurations->end(), temp_configs.begin(), temp_configs.end());
}


void defineConfigurations(
        const std::vector<uint64_t>& symbols,
        unsigned int wordSize,
        std::vector<Configuration> *configurations
){
    assert((wordSize == 1) || (wordSize == 2) | (wordSize == 4));
    assert(configurations != nullptr);

    configurations->clear();

    std::vector<uint64_t> transformedSymbols;
    std::vector<uint64_t> subsequence0;
    std::vector<uint64_t> subsequence1;
    std::vector<uint64_t> subsequence2;

    Configuration configuration;
    configuration.wordSize = wordSize;

    // Calculate entropy of the input symbolstream
    // TODO(Jan): check what happens if there are no symbols
    if (!symbols.empty())
    {
        double entropy = shannonEntropy(symbols);
        GABACIFY_LOG_DEBUG << "Entropy of the input symbols: " << std::fixed << std::setprecision(4) << entropy;
        size_t smallestCompressedSize = static_cast<size_t>(std::ceil((entropy / 8) * symbols.size()));
        GABACIFY_LOG_DEBUG << "Smallest compressed size assuming statistical independent symbols: " << std::fixed << smallestCompressedSize;
    }

    // Test all symbol transformations
    for (const auto& sequenceTransformationId : candidateSequenceTransformationIds)
    {
        configuration.sequenceTransformationId = sequenceTransformationId;
        switch (static_cast<unsigned int>(sequenceTransformationId))
        {
            // no_transform
            case static_cast<unsigned int>(gabac::SequenceTransformationId::no_transform):
                GABACIFY_LOG_INFO << "Defining configurations for sequence transformation 'no_transform'";
                configuration.transformedSequenceConfigurations =
                        std::vector<TransformedSequenceConfiguration>{TransformedSequenceConfiguration()};
                addSubsequences(
                        std::vector<std::vector<uint64_t>>{symbols},
                        std::vector<unsigned int>{wordSize},
                        configurations,
                        &configuration
                );
                break;
                // Equality Coding
            case static_cast<unsigned int>(gabac::SequenceTransformationId::equality_coding):
                GABACIFY_LOG_INFO << "Defining configurations for sequence transformation 'equality_coding'";
                configuration.transformedSequenceConfigurations =
                        std::vector<TransformedSequenceConfiguration>{
                                TransformedSequenceConfiguration(),
                                TransformedSequenceConfiguration()};
                gabac::transformEqualityCoding(
                        symbols,
                        &subsequence0,
                        &subsequence1
                );
                addSubsequences(
                        std::vector<std::vector<uint64_t>>{subsequence0, subsequence1},
                        std::vector<unsigned int>{1, wordSize},
                        configurations,
                        &configuration
                );
                break;
                // Match Coding
            case static_cast<unsigned int>(gabac::SequenceTransformationId::match_coding):
                GABACIFY_LOG_INFO << "Defining configurations for sequence transformation 'match_coding'";
                for (auto& matchCodingParameter : candidateMatchCodingParameters)
                {
                    configuration.sequenceTransformationParameter = matchCodingParameter;
                    configuration.transformedSequenceConfigurations =
                            std::vector<TransformedSequenceConfiguration>{
                                    TransformedSequenceConfiguration(),
                                    TransformedSequenceConfiguration(),
                                    TransformedSequenceConfiguration()};
                    gabac::transformMatchCoding(
                            symbols,
                            matchCodingParameter,
                            &subsequence0,
                            &subsequence1,
                            &subsequence2
                    );
                    addSubsequences(
                            std::vector<std::vector<uint64_t>>{subsequence0, subsequence1, subsequence2},
                            std::vector<unsigned int>{4, 4, wordSize},
                            configurations,
                            &configuration
                    );
                }
                break;
                // RLE coding
            case (static_cast<unsigned int>(gabac::SequenceTransformationId::rle_coding)):
                GABACIFY_LOG_INFO << "Defining configurations for sequence transformation 'rle_coding'";
                for (auto& rleCodingParameter : candidateRLECodingParameters)
                {
                    configuration.sequenceTransformationParameter = rleCodingParameter;
                    configuration.transformedSequenceConfigurations =
                            std::vector<TransformedSequenceConfiguration>{
                                    TransformedSequenceConfiguration(),
                                    TransformedSequenceConfiguration()};
                    gabac::transformRleCoding(
                            symbols,
                            rleCodingParameter,
                            &subsequence0,
                            &subsequence1
                    );
                    addSubsequences(
                            std::vector<std::vector<uint64_t>>{subsequence0, subsequence1},
                            std::vector<unsigned int>{wordSize, 4},
                            configurations,
                            &configuration
                    );
                }
                break;
            default:
                // TODO(Jan): return GABAC_FAILURE;
                break;
        }
    }
}


}  // namespace gabacify
