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
                         gabac::DataStream *const diffAndLutTransformedSequence,
                         gabac::OutputStream* const out
){
    // Encoding
    gabac::encode(
            conf.binarizationId,
            conf.binarizationParameters,
            conf.contextSelectionId,
            diffAndLutTransformedSequence
    );

    out->writeStream(diffAndLutTransformedSequence);
}

//------------------------------------------------------------------------------

void doLutTransform(bool enabled,
                    unsigned int order,
                    std::vector<gabac::DataStream> *const lutSequences,
                    unsigned *bits0,
                    gabac::OutputStream* out
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
    encodeStream({false, 0, 0, false, gabac::BinarizationId::BI, {*bits0}, gabac::ContextSelectionId::bypass}, &(*lutSequences)[1], out);

    if (order > 0) {
        bits1 = unsigned(std::ceil(std::log2(bits1)));
        encodeStream({false, 0, 0, false, gabac::BinarizationId::BI, {bits1}, gabac::ContextSelectionId::bypass}, &(*lutSequences)[2], out);
    }

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
                                 gabac::DataStream *const seq,
                                 gabac::OutputStream* const out
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
            &bits,
            out
    );

    // Put lut transformed in, get difftransformed out
    doDiffTransform(
            configuration.diffCodingEnabled,
            &lutTransformedSequences[0]
    );

    encodeStream(configuration, &lutTransformedSequences[0], out);
}

//------------------------------------------------------------------------------

static void encodeWithConfiguration(
        const Configuration& configuration,
        gabac::InputStream* inStream,
        gabac::OutputStream* outStream
){
    const size_t BLOCKSIZE = 16000000;
    while(inStream->isValid()) {
        gabac::DataStream sequence(0, 1);
        size_t readLength = std::min(BLOCKSIZE, inStream->getRemainingSize());
        inStream->readBytes(readLength, &sequence);
        sequence.setWordSize(configuration.wordSize);
        // Insert sequence into vector
        std::vector<gabac::DataStream> transformedSequences;
        transformedSequences.resize(1);
        transformedSequences[0].swap(&sequence);

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
                    &(transformedSequences[i]),
                    outStream
            );
        }
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
    gabac::FileInputStream instream(inputFile.handle());

    // Write the bytestream
    OutputFile outputFile(outputFilePath);
    gabac::FileOutputStream outstream(outputFile.handle());

    // Convert Symbol stream to bytestream
    encodeWithConfiguration(configuration, &instream, &outstream);

    GABACIFY_LOG_INFO << "Wrote bytestream of size "
                      << outstream.bytesWritten()
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
