#include "gabacify/encode.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iomanip>
#include <limits>
#include <utility>
#include <vector>

#include "gabac/constants.h"
#include "gabac/encoding.h"
#include "gabac/diff_coding.h"

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
        const gabac::DataStream& bytes,
        gabac::DataStream *const bytestream
){
    assert(bytestream != nullptr);

    uint32_t size = bytes.size() * bytes.getWordSize();
    uint8_t *ptr = (uint8_t *) &size;
    bytestream->push_back(ptr[0]);
    bytestream->push_back(ptr[1]);
    bytestream->push_back(ptr[2]);
    bytestream->push_back(ptr[3]);

    // Append 'bytes' to the bytestream
    bytestream->insert(bytestream->end(), bytes.begin(), bytes.end());
}

//------------------------------------------------------------------------------

void doSequenceTransform(const gabac::SequenceTransformationId& transID,
                         uint64_t param,
                         std::vector<gabac::DataStream> *const transformedSequences
){
    GABACIFY_LOG_TRACE << "Encoding sequence of length: " << (*transformedSequences)[0].size();

    auto id = unsigned(transID);
    GABACIFY_LOG_DEBUG << "Performing sequence transformation " << gabac::transformationInformation[id].name;

    gabac::transformationInformation[id].transform(param, transformedSequences);

    GABACIFY_LOG_TRACE << "Got " << transformedSequences->size() << " sequences";
    for (unsigned i = 0; i < transformedSequences->size(); ++i) {
        GABACIFY_LOG_TRACE << i << ": " << (*transformedSequences)[i].size() << " bytes";
    }
}

//------------------------------------------------------------------------------

static void encodeStream(const TransformedSequenceConfiguration& conf,
                         gabac::DataStream *const diffAndLutTransformedSequence
){
    // Encoding
    gabac::encode(
            conf.binarizationId,
            conf.binarizationParameters,
            conf.contextSelectionId,
            diffAndLutTransformedSequence
    );
}

//------------------------------------------------------------------------------

void doLutTransform(bool enabled,
                    unsigned int order,
                    std::vector<gabac::DataStream> *const lutSequences,
                    unsigned *bits0
){
    if (!enabled) {
        GABACIFY_LOG_TRACE << "LUT transform *dis*abled";
        GABACIFY_LOG_DEBUG << "Got uncompressed stream after LUT: " << 0 << " bytes";
        GABACIFY_LOG_DEBUG << "Got table after LUT: " << 0 << " bytes";
        return;
    }

    GABACIFY_LOG_TRACE << "LUT transform *en*abled";
    const unsigned LUT_INDEX = 4;

    // Put raw sequence in, get transformed sequence and lut tables
    gabac::transformationInformation[LUT_INDEX].transform(order, lutSequences);

    if ((*lutSequences)[0].empty()) {
        return;
    }

    GABACIFY_LOG_DEBUG << "Got uncompressed stream after LUT: " << (*lutSequences)[0].size() << " bytes";
    GABACIFY_LOG_DEBUG << "Got table0 after LUT: " << (*lutSequences)[1].size() << " bytes";
    GABACIFY_LOG_DEBUG << "Got table1 after LUT: " << (*lutSequences)[2].size() << " bytes";

    // Calculate bit size for order 0 table
    if (*bits0 == 0) {
        uint64_t min = 0, max = 0;
        deriveMinMaxUnsigned(lutSequences->at(1), sizeof(uint64_t), &min, &max);
        *bits0 = unsigned(std::ceil(std::log2(max + 1)));
        if (max <= 1) {
            *bits0 = 1;
        }
    }

    unsigned bits1 = unsigned((*lutSequences)[1].size());
    encodeStream({false, 0, 0, false, gabac::BinarizationId::BI, {*bits0}, gabac::ContextSelectionId::bypass}, &(*lutSequences)[1]);

    if (order > 0) {
        bits1 = unsigned(std::ceil(std::log2(bits1)));
        encodeStream({false, 0, 0, false, gabac::BinarizationId::BI, {bits1}, gabac::ContextSelectionId::bypass}, &(*lutSequences)[2]);
    }

    GABACIFY_LOG_TRACE << "Wrote LUT bitstream0 with size: " << (*lutSequences)[1].size();
    GABACIFY_LOG_TRACE << "Wrote LUT bitstream1 with size: " << (*lutSequences)[2].size();
}

//------------------------------------------------------------------------------

void doDiffTransform(bool enabled,
                     gabac::DataStream *const diffAndLutTransformedSequence
){
    // Diff coding
    if (enabled) {
        GABACIFY_LOG_TRACE << "Diff coding *en*abled";
        gabac::transformDiffCoding(diffAndLutTransformedSequence);
        GABACIFY_LOG_DEBUG << "Got uncompressed stream after diff: "
                           << diffAndLutTransformedSequence->size()
                           << " bytes";
        return;
    }


    GABACIFY_LOG_TRACE << "Diff coding *dis*abled";
    GABACIFY_LOG_DEBUG << "Got uncompressed stream after diff: " << diffAndLutTransformedSequence->size() << " bytes";
}

//------------------------------------------------------------------------------

static void encodeSingleSequence(const TransformedSequenceConfiguration& configuration,
                                 gabac::DataStream *const seq
){
    std::vector<gabac::DataStream> lutTransformedSequences;

    // Symbol/transformed symbols, lut0 bytestream, lut1 bytestream
    lutTransformedSequences.resize(1);
    lutTransformedSequences[0].swap(seq);

    // Put sequence in, get lut sequence and lut bytestreams
    unsigned bits = configuration.lutBits;
    doLutTransform(
            configuration.lutTransformationEnabled,
            configuration.lutOrder,
            &lutTransformedSequences,
            &bits
    );
    if(configuration.lutTransformationEnabled) {
        appendToBytestream(lutTransformedSequences[1], seq);
        lutTransformedSequences[1].clear();
        lutTransformedSequences[1].shrink_to_fit();
        if (configuration.lutOrder > 0) {
            appendToBytestream(lutTransformedSequences[2], seq);
            lutTransformedSequences[2].clear();
            lutTransformedSequences[2].shrink_to_fit();
        }
    }

    lutTransformedSequences.resize(1);

    // Put lut transformed in, get difftransformed out
    doDiffTransform(
            configuration.diffCodingEnabled,
            &lutTransformedSequences[0]
    );

    encodeStream(configuration, &lutTransformedSequences[0]);

    appendToBytestream(lutTransformedSequences[0], seq);
}

//------------------------------------------------------------------------------

static void encodeWithConfiguration(
        const Configuration& configuration,
        gabac::DataStream *const sequence
){
    // Insert sequence into vector
    std::vector<gabac::DataStream> transformedSequences;
    transformedSequences.resize(1);
    transformedSequences[0].swap(sequence);

    // Put symbol stream in, get transformed streams out
    doSequenceTransform(
            configuration.sequenceTransformationId,
            configuration.sequenceTransformationParameter,
            &transformedSequences
    );

    // Loop through the transformed sequences
    for (size_t i = 0; i < transformedSequences.size(); i++) {
        // Put transformed sequence in, get partial bytestream back
        encodeSingleSequence(
                configuration.transformedSequenceConfigurations[i],
                &(transformedSequences[i])
        );

        // Extend bytestream & free sequence
        appendToBytestream(transformedSequences[i], sequence);
        transformedSequences[i].clear();
        transformedSequences[i].shrink_to_fit();
    }

}

//------------------------------------------------------------------------------

void encode_plain(const std::string& inputFilePath,
                  const std::string& configurationFilePath,
                  const std::string& outputFilePath
){
    InputFile configurationFile(configurationFilePath);
    std::string jsonInput("\0", configurationFile.size());
    configurationFile.read(&jsonInput[0], 1, jsonInput.size());
    Configuration configuration(jsonInput);

    // Read in the entire input file
    InputFile inputFile(inputFilePath);
    gabac::DataStream symbols(inputFile.size() / configuration.wordSize, configuration.wordSize);
    inputFile.read(symbols.getData(), 1, symbols.size() * configuration.wordSize);

    // Convert Symbol stream to bytestream
    encodeWithConfiguration(configuration, &symbols);

    // Write the bytestream
    OutputFile outputFile(outputFilePath);
    outputFile.write(symbols.getData(), 1, symbols.size() * symbols.getWordSize());
    GABACIFY_LOG_INFO << "Wrote bytestream of size "
                      << symbols.size() * symbols.getWordSize()
                      << " to: "
                      << outputFilePath;
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

    if (analyze) {
        encode_analyze(inputFilePath, configurationFilePath, outputFilePath);
        return;
    }
    encode_plain(inputFilePath, configurationFilePath, outputFilePath);
}

//------------------------------------------------------------------------------

}  // namespace gabacify

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
